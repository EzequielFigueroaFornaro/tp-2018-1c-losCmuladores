/*
 ============================================================================
 Name        : planifier.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto planifier.
 ============================================================================
 */

#include "orchestrator.h"

#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>

long id = 0;
long cpu_time = 0;

long RUNNING_ESI = 0;
long NEXT_RUNNING_ESI = 0;


/*aca se pueden tener un par de funciones globales para todas las esis
 * ejemplo, podemos tener un mapa de esis por id y el puntero a la esi, desde ahi podemos modificarlas mas rapidametne
 * usando el puntero en el mapa, en vez de ir ala lista correspondinte a tener que buscarla */

void replan_by_algorithm() {
	switch (algorithm) {
		case FIFO: fifo_replan(); break;
		case SJF: sjf_replan(); break;
		case SJF_DESALOJO: sjf_desa_replan(); break;
		case HRRN: hrrn_replan(); break;
		default: fifo_replan();	break;
	}
}

void notify_dispatcher() {
	if (RUNNING_ESI == 0) {
		replan_by_algorithm();
		pthread_mutex_unlock(&dispatcher_manager);
	}
}

void set_orchestrator() {
	esi_map = dictionary_create();
	esis_bloqueados_por_recurso = dictionary_create();
	recurso_tomado_por_esi = dictionary_create();
	READY_ESI_LIST = list_create();
	BLOCKED_ESI_LIST = list_create();
	FINISHED_ESI_LIST = queue_create();
}

long increment_id() {
	pthread_mutex_lock(&id_mtx);
	id++;
	pthread_mutex_unlock(&id_mtx);
	return id;
}

long cpu_time_incrementate(){
	pthread_mutex_lock(&cpu_time_mtx);
	long new_cpu_time = cpu_time ++;
	pthread_mutex_unlock(&cpu_time_mtx);
	return new_cpu_time;
}

long get_current_time() {
	pthread_mutex_lock(&cpu_time_mtx);
	long time = cpu_time;
	pthread_mutex_unlock(&cpu_time_mtx);
	return time;
}

void add_esi_by_algorithm(esi* esi) {
	switch(algorithm) {
		case FIFO: fifo_add_esi(esi->id); break;
		case SJF: sjf_add_esi(esi->id); break;
		case SJF_DESALOJO: sjf_desa_add_esi(esi->id); break;
		case HRRN: hrrn_add_esi(esi->id); break;
		default: fifo_add_esi(esi->id);	break;
	}
}

void add_esi(esi* esi){
	pthread_mutex_lock(&esi_map_mtx_6);
	char* key = id_to_string(esi->id);
	dictionary_put(esi_map, key, esi);
	//log_debug(logger, "Status of ESIs: %s", esis_to_string());
	pthread_mutex_unlock(&esi_map_mtx_6);
	add_esi_by_algorithm(esi);
}

bool is_valid_esi(long esi_id){
	pthread_mutex_lock(&esi_map_mtx_6);
	bool result = dictionary_has_key(esi_map,id_to_string(esi_id)) || dictionary_is_empty(esi_map);
	pthread_mutex_unlock(&esi_map_mtx_6);
	return result;
}

bool esi_exists(long esi_id) {
	pthread_mutex_lock(&esi_map_mtx_6);
	bool result = dictionary_has_key(esi_map,id_to_string(esi_id));
	pthread_mutex_unlock(&esi_map_mtx_6);
	return result;
}

void modificar_estado(long esi_id, estado nuevo_estado){
	log_debug(logger, "Changing ESI%ld's state to %d", esi_id, nuevo_estado);
	pthread_mutex_lock(&esi_map_mtx_6);
	esi* selected_esi = (esi*) dictionary_get(esi_map, id_to_string(esi_id));
	selected_esi -> estado = nuevo_estado;
//	log_debug(logger, "Status of all ESIs after modifying status of ESI%ld: %s", esi_id, esis_to_string());
	pthread_mutex_unlock(&esi_map_mtx_6);
}

void block_esi(long esi_id){
	modificar_estado(esi_id, BLOQUEADO);

	pthread_mutex_lock(&esi_map_mtx_6);
	esi* selected_esi = (esi*) dictionary_get(esi_map, id_to_string(esi_id));
	selected_esi -> duracion_real_ultima_rafaga = selected_esi -> instruction_pointer;
	log_debug(logger, "Duracion ultima rafaga: %d", selected_esi -> duracion_real_ultima_rafaga);
	pthread_mutex_unlock(&esi_map_mtx_6);

	switch(algorithm) {
		case FIFO: fifo_block_esi(esi_id); break;
		case SJF: sjf_block_esi(esi_id); break;
		case SJF_DESALOJO: sjf_desa_block_esi(esi_id); break;
		case HRRN: hrrn_block_esi(esi_id); break;
		default: fifo_block_esi(esi_id); break;
	}
}

void unblock_esi(long esi_id){
	pthread_mutex_lock(&blocked_list_mtx_3);
	modificar_estado(esi_id, DESBLOQUEADO);
	list_remove_esi(BLOCKED_ESI_LIST, esi_id);
	pthread_mutex_unlock(&blocked_list_mtx_3);

	add_esi_by_algorithm(get_esi_by_id(esi_id));

	notify_dispatcher();
}

void finish_esi(long esi_id){
	log_debug(logger, "Moving ESI%ld to finished list...", esi_id);
	pthread_mutex_lock(&esi_map_mtx_6);
	esi* selected_esi = (esi*) dictionary_get(esi_map, id_to_string(esi_id));
	pthread_mutex_unlock(&esi_map_mtx_6);
	//TODO el terminar un esi exigue liberar los recursos que tien etomados?
	switch(selected_esi -> estado) {
		case BLOQUEADO:
			pthread_mutex_lock(&blocked_list_mtx_3);
			list_remove_esi(BLOCKED_ESI_LIST, selected_esi -> id);
			pthread_mutex_unlock(&blocked_list_mtx_3);
			break;
		case CORRIENDO:
			fifo_finish_esi();
			break;
		default:
			pthread_mutex_lock(&ready_list_mtx_4);
			list_remove_esi(READY_ESI_LIST, selected_esi -> id);
			pthread_mutex_unlock(&ready_list_mtx_4);
			break;
	}
	modificar_estado(selected_esi -> id, FINALIZADO);

	pthread_mutex_lock(&finished_list_mtx_5);
	queue_push_id(FINISHED_ESI_LIST, selected_esi ->id);
	pthread_mutex_unlock(&finished_list_mtx_5);

	t_list* resources_taken = get_resources_taken_by_esi(selected_esi->id);
	list_iterate(resources_taken, (void*) free_resource);
	list_destroy(resources_taken);
}

long esi_se_va_a_ejecutar(){
	pthread_mutex_lock(&running_esi_mtx_1);
	pthread_mutex_lock(&next_running_esi_mtx_2);

	if ((RUNNING_ESI == NEXT_RUNNING_ESI) && NEXT_RUNNING_ESI!=0) {
		pthread_mutex_lock(&esi_map_mtx_6);
		esi* selected_esi = (esi*) dictionary_get(esi_map, id_to_string(RUNNING_ESI));
		pthread_mutex_unlock(&esi_map_mtx_6);
		selected_esi -> instruction_pointer++;
	} else {
		RUNNING_ESI = NEXT_RUNNING_ESI;
		if(RUNNING_ESI != 0) {
			modificar_estado(RUNNING_ESI, CORRIENDO);
		}
	}

	pthread_mutex_unlock(&next_running_esi_mtx_2);
	pthread_mutex_unlock(&running_esi_mtx_1);
	return RUNNING_ESI;
}

void borrado_de_finish(){
	pthread_mutex_lock(&finished_list_mtx_5);
	log_debug(logger, "Deleting finished ESIs... Found %d to delete: [%s]", queue_size(FINISHED_ESI_LIST), list_join(FINISHED_ESI_LIST->elements));
	while(!queue_is_empty(FINISHED_ESI_LIST)){
		long* esi_to_be_freed = queue_pop(FINISHED_ESI_LIST);

		pthread_mutex_lock(&esi_map_mtx_6);
		esi* selected_esi = (esi*) dictionary_remove(esi_map, id_to_string(*esi_to_be_freed)); /*como mierda liberar el espacio del esi*/
		free(selected_esi);
		pthread_mutex_unlock(&esi_map_mtx_6);
	}
	pthread_mutex_unlock(&finished_list_mtx_5);
}

t_queue* get_all_waiting_for_resource(char* resource) {
	return dictionary_get(esis_bloqueados_por_recurso, resource);
}

void block_esi_by_resource(long esi_id, char* resource) {
	block_esi(esi_id);
	log_debug(logger, "Esi%ld bloqueado...", esi_id);
	pthread_mutex_lock(&blocked_by_resource_map_mtx);
	t_queue* blocked_esis = get_all_waiting_for_resource(resource);
	if (blocked_esis == NULL) {
		blocked_esis = queue_create();
	}
	queue_push_id(blocked_esis, esi_id);
	dictionary_put(esis_bloqueados_por_recurso, resource, blocked_esis);

	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
}

char* get_all_waiting_for_resource_as_string(char* resource, char* separator) {
	pthread_mutex_lock(&blocked_by_resource_map_mtx);

	if (esis_bloqueados_por_recurso != NULL) {
		t_queue* esis_blocked = get_all_waiting_for_resource(resource);
		if (esis_blocked != NULL) {
			char* buffer = string_new();
			int elements_count = queue_size(esis_blocked);
			int index = 0;
			void to_string(long* esi_id) {
				index++;
				if (index == elements_count) {
					string_append_with_format(&buffer, "ESI%ld", *esi_id);
				} else {
					string_append_with_format(&buffer, "ESI%ld%s", *esi_id, separator);
				}
			}
			list_iterate(esis_blocked->elements, (void*) to_string);
			pthread_mutex_unlock(&blocked_by_resource_map_mtx);
			return buffer;
		}
	}
	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
	return "";
}

bool bloquear_recurso(char* recurso, long esi_id) {
	bool able_to_give_resource;
	log_debug(logger, "Bloqueando...");
	pthread_mutex_lock(&blocked_resources_map_mtx);
	if (resource_taken(recurso, esi_id)) {
		log_debug(logger, "Recurso tomado, agregando a lista de bloqueados...");
		block_esi_by_resource(esi_id, recurso);
		log_debug(logger, "Agregado a lista de bloqueados...");
		cambiar_recurso_que_lo_bloquea(recurso,esi_id);
		able_to_give_resource = false;
	} else {
		dictionary_put_id(recurso_tomado_por_esi, recurso, esi_id);
		able_to_give_resource = true;
	}
	pthread_mutex_unlock(&blocked_resources_map_mtx);
	return able_to_give_resource;
}

bool resource_taken_by_any_esi(char* resource) {
	return dictionary_has_key(recurso_tomado_por_esi, resource);
}


void cambiar_recurso_que_lo_bloquea(char* recurso, long esi_id){
	log_debug(logger, "Changing ESI%ld's blocking resource to '%s'", esi_id, recurso);
	pthread_mutex_lock(&esi_map_mtx_6);
	esi* selected_esi = (esi*) dictionary_get(esi_map, id_to_string(esi_id));
	memcpy(selected_esi -> blocking_resource , recurso, strlen(recurso) + 1); //TODO ver si hay que hacer free de RECURSO.
//    log_debug(logger, "Status of all ESIs after modifying status of ESI%ld: %s", esi_id, esis_to_string());
    pthread_mutex_unlock(&esi_map_mtx_6);
}

pthread_mutex_t DEADLOCK_ENCONTRADO_MUTEX = PTHREAD_MUTEX_INITIALIZER;
bool DEADLOCK_ENCONTRADO = false;


bool resource_taken(char* resource, long esi_id) {
	long* esi = dictionary_get(recurso_tomado_por_esi, resource);
	return esi != NULL && *esi != esi_id;
}

t_list* buscar_deadlock(){
	t_list* resultado = list_create();
	pthread_mutex_lock(&blocked_list_mtx_3);
	pthread_mutex_lock(&esi_map_mtx_6);
	pthread_mutex_lock(&blocked_by_resource_map_mtx);
	pthread_mutex_lock(&DEADLOCK_ENCONTRADO_MUTEX);
	for(int i=0; i<list_size(BLOCKED_ESI_LIST); i++){
		long* esi_id = list_get(BLOCKED_ESI_LIST , i);
		log_debug(logger, "Buscando dependencia circular para ESI id: %lu", *esi_id);
		DEADLOCK_ENCONTRADO = false;
		t_list* bloqueados = buscar_deadlock_en_lista(*esi_id, list_create());
		for(int j=0; j<list_size(bloqueados); j++) {
			long* id = list_get(bloqueados , j);
			bool id_function(long* list_id){
				return *list_id == *id;
			}
			if(!list_any_satisfy(resultado, (void*)id_function)){
				list_add_id(resultado, *id);
			}
		}
	}
	pthread_mutex_unlock(&DEADLOCK_ENCONTRADO_MUTEX);
	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
	pthread_mutex_unlock(&esi_map_mtx_6);
	pthread_mutex_unlock(&blocked_list_mtx_3);
	return resultado;
}

t_list* buscar_deadlock_en_lista(long id, t_list* corte){

	bool id_function(long* list_id){
		return *list_id == id;
	}

	if(list_any_satisfy(corte,(void*)id_function)){
		t_list* ids_en_deadlock = list_create();
		list_add_id(ids_en_deadlock, id);
		return ids_en_deadlock;
	} else {
		esi* _esi = (esi*) dictionary_get(esi_map, id_to_string(id));
		if(_esi == NULL || (_esi->estado) != BLOQUEADO){
			return list_create();
		}

		char* recurso = _esi -> blocking_resource;
		long* esi_bloqueante_id = (long*) dictionary_get(recurso_tomado_por_esi, recurso);
		log_debug(logger, "Esi bloqueante para %lu es el ESI %lu", id, *esi_bloqueante_id);
		list_add_id(corte, id);
		t_list* resultado = buscar_deadlock_en_lista(*esi_bloqueante_id, corte);
		if(list_is_empty(resultado) || list_any_satisfy(resultado, (void*)id_function) || DEADLOCK_ENCONTRADO){
			DEADLOCK_ENCONTRADO = true;
			return resultado;
		} else {
			log_debug(logger, "Dependencia circular encontrada con ESI: %lu", id);
			list_add_id(resultado, id);
			return resultado;
		}
	}
}

void free_resource(char* resource) {
	log_debug(logger, "Liberando recurso %s...", resource);
	pthread_mutex_lock(&blocked_by_resource_map_mtx);
	pthread_mutex_lock(&blocked_resources_map_mtx);
	//TODO ver que onda desbloqueo todo o una sola. UPDATE: habiamos quedado en desbloquear solo una, no?
	dictionary_remove(recurso_tomado_por_esi, resource);
	t_queue* esi_queue = dictionary_get(esis_bloqueados_por_recurso, resource);
	if(esi_queue != NULL && !queue_is_empty(esi_queue)){
		long* esi_id = queue_pop(esi_queue);
		while (!is_valid_esi(*esi_id)) {
			esi_id = queue_pop(esi_queue);
		}
		cambiar_recurso_que_lo_bloquea("", *esi_id);
		pthread_mutex_unlock(&blocked_resources_map_mtx);
		pthread_mutex_unlock(&blocked_by_resource_map_mtx);
		unblock_esi(*esi_id);
	}else{
		pthread_mutex_unlock(&blocked_resources_map_mtx);
		pthread_mutex_unlock(&blocked_by_resource_map_mtx);
	}
	//TODO OJO AL PIOJO el free de datos como el id que guardamos de la esi bloqueada;
}

float estimate_next_cpu_burst(esi* esi) {
	float estimated_cpu_burst;
	if (esi->estado == NUEVO) {
		estimated_cpu_burst = initial_estimation;
	} else if (esi->estado == DESBLOQUEADO) {
		float alpha_coef = alpha / 100;
		float last_cpu_burst_coef = esi->duracion_real_ultima_rafaga * alpha_coef;
		float last_estimated_cpu_burst_coef = (1 - alpha_coef) * esi->estimacion_ultima_rafaga;
		estimated_cpu_burst = last_cpu_burst_coef + last_estimated_cpu_burst_coef;
	} else if (esi->estado == CORRIENDO) {
		estimated_cpu_burst = esi->estimacion_ultima_rafaga - esi->duracion_real_ultima_rafaga;
	} else {
		log_error(logger, "ESTO NO DEBERIA PASAR!!");
	}
	esi->estimacion_ultima_rafaga = estimated_cpu_burst;
	log_debug(logger, "Rafaga estimada para el ESI%ld: %2.5f", esi->id, estimated_cpu_burst);
	return estimated_cpu_burst;
}
