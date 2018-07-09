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

long id = 0;
long cpu_time = 0;

long RUNNING_ESI = 0;
long NEXT_RUNNING_ESI = 0;


/*aca se pueden tener un par de funciones globales para todas las esis
 * ejemplo, podemos tener un mapa de esis por id y el puntero a la esi, desde ahi podemos modificarlas mas rapidametne
 * usando el puntero en el mapa, en vez de ir ala lista correspondinte a tener que buscarla */

void replan_by_algorithm() {
	switch (algorithm) {
	case FIFO:
		fifo_replan();
		break;
	default:
		fifo_replan();
		break;
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
	pthread_mutex_trylock(&id_mtx);
	id++;
	pthread_mutex_unlock(&id_mtx);
	return id;
}

long cpu_time_incrementate(){
	pthread_mutex_trylock(&cpu_time_mtx);
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

void add_esi(esi* esi){
	pthread_mutex_lock(&esi_map_mtx);
	dictionary_put(esi_map,id_to_string(esi->id), esi);
	log_debug(logger, "Status of ESIs: %s", esis_to_string());
	pthread_mutex_unlock(&esi_map_mtx);
	switch(algorithm) {
		case FIFO:
			fifo_add_esi(esi->id);
			break;
		default:
			fifo_add_esi(esi->id);
			break;
	}
}

bool is_valid_esi(long esi_id){
	pthread_mutex_lock(&esi_map_mtx);
	bool result = dictionary_has_key(esi_map,id_to_string(esi_id)) || dictionary_is_empty(esi_map);
	pthread_mutex_unlock(&esi_map_mtx);
	return result;
}

bool esi_exists(long esi_id) {
	pthread_mutex_lock(&esi_map_mtx);
	bool result = dictionary_has_key(esi_map,id_to_string(esi_id));
	pthread_mutex_unlock(&esi_map_mtx);
	return result;
}

void modificar_estado(long esi_id, estado nuevo_estado){
	log_debug(logger, "Changing ESI%ld's state to %d", esi_id, nuevo_estado);
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_get(esi_map, id_to_string(esi_id));
	esi -> estado = nuevo_estado;
	log_debug(logger, "Status of all ESIs after modifying status of ESI%ld: %s", esi_id, esis_to_string());
	pthread_mutex_unlock(&esi_map_mtx);
}

void block_esi(long esi_id){
	modificar_estado(esi_id, BLOQUEADO);
	switch(algorithm) {
			case FIFO:
				fifo_block_esi(esi_id);
				break;
			default:
				fifo_block_esi(esi_id);
				break;
		}
}

void unblock_esi(long esi_id){
	pthread_mutex_lock(&blocked_list_mtx);
	modificar_estado(esi_id, LISTO);
	list_remove_esi(BLOCKED_ESI_LIST, esi_id);
	pthread_mutex_unlock(&blocked_list_mtx);
	switch(algorithm) {
		case FIFO:
			fifo_add_esi(esi_id);
			break;
		default:
			fifo_add_esi(esi_id);
			break;
	}
	notify_dispatcher();
}

void finish_esi(long esi_id){
	log_debug(logger, "Moving ESI%ld to finished list...", esi_id);
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_get(esi_map, id_to_string(esi_id));
	pthread_mutex_unlock(&esi_map_mtx);

	switch(esi->estado) {
		case BLOQUEADO:
			pthread_mutex_lock(&blocked_list_mtx);
			list_remove_esi(BLOCKED_ESI_LIST, esi->id);
			pthread_mutex_unlock(&blocked_list_mtx);
			break;
		case CORRIENDO:
			fifo_finish_esi();
			break;
		default:
			pthread_mutex_lock(&ready_list_mtx);
			list_remove_esi(READY_ESI_LIST, esi->id);
			pthread_mutex_unlock(&ready_list_mtx);
			break;
	}
	modificar_estado(esi->id, FINALIZADO);

	pthread_mutex_lock(&finished_list_mtx);
	queue_push_id(FINISHED_ESI_LIST, esi->id);
	pthread_mutex_unlock(&finished_list_mtx);
}

long esi_se_va_a_ejecutar(){
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);

	if (RUNNING_ESI == NEXT_RUNNING_ESI) {
		pthread_mutex_lock(&esi_map_mtx);
		esi* esi = dictionary_get(esi_map, id_to_string(RUNNING_ESI));
		pthread_mutex_unlock(&esi_map_mtx);
		esi->instruction_pointer++;
	} else {
		RUNNING_ESI = NEXT_RUNNING_ESI;
		if(RUNNING_ESI != 0) {
			modificar_estado(RUNNING_ESI, CORRIENDO);
		}
	}

	pthread_mutex_unlock(&next_running_esi_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	return RUNNING_ESI;
}

void borrado_de_finish(){
	pthread_mutex_lock(&finished_list_mtx);
	log_debug(logger, "Deleting finished ESIs... Found %d to delete: [%s]", queue_size(FINISHED_ESI_LIST), list_join(FINISHED_ESI_LIST->elements));
	while(!queue_is_empty(FINISHED_ESI_LIST)){
		long* esi_to_be_freed = queue_pop(FINISHED_ESI_LIST);

		pthread_mutex_lock(&esi_map_mtx);
		esi* esi = dictionary_remove(esi_map, id_to_string(*esi_to_be_freed)); /*como mierda liberar el espacio del esi*/
		free(esi);
		pthread_mutex_unlock(&esi_map_mtx);
	}
	pthread_mutex_unlock(&finished_list_mtx);
}

t_queue* get_all_waiting_for_resource(char* resource) {
	return dictionary_get(esis_bloqueados_por_recurso, resource);
}

void block_esi_by_resource(long esi_id, char* resource) {
	block_esi(esi_id);
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

	pthread_mutex_lock(&blocked_resources_map_mtx);
	if (resource_taken(recurso)) {
		block_esi_by_resource(esi_id, recurso);
		able_to_give_resource = false;
	} else {
		dictionary_put_id(recurso_tomado_por_esi, recurso, esi_id);
		able_to_give_resource = true;
	}
	pthread_mutex_unlock(&blocked_resources_map_mtx);
	return able_to_give_resource;
}


char* get_resource_taken_by_esi(long esi_id) {
	char* resource = "";

	void find_resource(char* key, long* value) {
		if (esi_id == *value) {
			resource = key;
		}
	}
	pthread_mutex_lock(&blocked_resources_map_mtx);
	if (dictionary_is_empty(recurso_tomado_por_esi)) {
		pthread_mutex_unlock(&blocked_resources_map_mtx);
		return "";
	}
	dictionary_iterator(recurso_tomado_por_esi, (void*) find_resource);
	pthread_mutex_unlock(&blocked_resources_map_mtx);
	return resource;
}

bool resource_taken(char* resource) {
	return dictionary_has_key(recurso_tomado_por_esi, resource);
}
