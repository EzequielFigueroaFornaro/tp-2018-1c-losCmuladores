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

long RUNNING_ESI = 0;
long NEXT_RUNNING_ESI = -1;

void set_orchestrator(int algorithm) {
	ALGORITHM = algorithm;
	READY_ESI_LIST = list_create();
	BLOCKED_ESI_LIST = list_create();
	FINISHED_ESI_LIST = queue_create();
}

/*aca se pueden tener un par de funciones globales para todas las esis
 * ejemplo, podemos tener un mapa de esis por id y el puntero a la esi, desde ahi podemos modificarlas mas rapidametne
 * usando el puntero en el mapa, en vez de ir ala lista correspondinte a tener que buscarla */

char* esis_to_string() {
	char* buffer = string_new();
	void to_string(char* esi_id, esi* esi) {
		string_append_with_format(&buffer, "\n\t\t\t\t\t\tESI%ld -> status: %d", esi->id, esi->estado);
	}
	dictionary_iterator(esi_map, (void*)to_string);
	return buffer;
}

void add_esi(esi* esi){
	pthread_mutex_lock(&esi_map_mtx);
	dictionary_put(esi_map,string_key(esi->id), esi);
	log_debug(logger, "Status of ESIs: %s", esis_to_string());
	pthread_mutex_unlock(&esi_map_mtx);
	switch(ALGORITHM) {
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
	bool result = dictionary_has_key(esi_map,string_key(esi_id)) || dictionary_is_empty(esi_map);
	pthread_mutex_unlock(&esi_map_mtx);
	return result;
}

void modificar_estado(long esi_id, int nuevo_estado){
	log_debug(logger, "Changing ESI%ld's state to %d", esi_id, nuevo_estado);
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_get(esi_map, string_key(esi_id));
	esi -> estado = nuevo_estado;
	log_debug(logger, "Status of all ESIs after modifying status of ESI%ld: %s", esi_id, esis_to_string());
	pthread_mutex_unlock(&esi_map_mtx);
}



void block_esi(long esi_id){
	switch(ALGORITHM) {
			case FIFO:
				fifo_block_esi(esi_id);
				break;
			default:
				fifo_block_esi(esi_id);
				break;
		}
}

void unlock_esi(long esi_id){
	bool equals_esi (long esi) {
		  return esi_id == esi;
	}
	pthread_mutex_lock(&blocked_list_mtx);
	modificar_estado(esi_id, DESBLOQUEADO);
	list_remove_by_condition(BLOCKED_ESI_LIST, (void*) equals_esi);
	pthread_mutex_unlock(&blocked_list_mtx);
	switch(ALGORITHM) {
			case FIFO:
				fifo_add_esi(esi_id);
				break;
			default:
				fifo_add_esi(esi_id);
				break;
		}
}

void finish_esi(long esi_id){
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_get(esi_map, string_key(esi_id));
	int estado_actual = esi -> estado;
	bool equals_esi (int esi_id) {
		return esi_id == estado_actual;
	}
	pthread_mutex_unlock(&esi_map_mtx);
	switch(estado_actual) {
		case BLOQUEADO:
			pthread_mutex_lock(&blocked_list_mtx);
			list_remove_by_condition(BLOCKED_ESI_LIST, (void*) equals_esi);
			pthread_mutex_unlock(&blocked_list_mtx);
			break;
		case CORRIENDO:
			fifo_finish_esi();
			break;
		default:
			pthread_mutex_lock(&ready_list_mtx);
			list_remove_by_condition(READY_ESI_LIST, (void*) equals_esi);
			pthread_mutex_unlock(&ready_list_mtx);
			break;
	}
	put_finish_esi(esi_id);
}

void free_esi(long esi_id){
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_remove(esi_map, string_key(esi_id)); /*como mierda liberar el espacio del esi*/
	free(esi);
	pthread_mutex_unlock(&esi_map_mtx);
}

char* string_key(long key){
	return string_from_format("%ld",key);
}

void add_esi_bloqueada(long esi_id){
	//TODO no me acuerdo que hacia aca
}

bool es_caso_base(long esi_id){
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	bool resut = NEXT_RUNNING_ESI == -1;
	NEXT_RUNNING_ESI = esi_id;
	pthread_mutex_unlock(&next_running_esi_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	return resut;
}

void volver_caso_base(){
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	RUNNING_ESI = -1;
	NEXT_RUNNING_ESI = 0;
	pthread_mutex_unlock(&next_running_esi_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
}

long esi_se_va_a_ejecutar(){
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	if(RUNNING_ESI == NEXT_RUNNING_ESI){
		pthread_mutex_lock(&esi_map_mtx);
		esi* esi = dictionary_get(esi_map, string_key(RUNNING_ESI));
		esi -> instrucction_pointer = ((esi -> instrucction_pointer) +1);
		if((esi -> instrucction_pointer) == (esi -> cantidad_de_instrucciones)){
			pthread_mutex_unlock(&esi_map_mtx);
			finish_esi(RUNNING_ESI);
		}
	}
	RUNNING_ESI = NEXT_RUNNING_ESI;
	pthread_mutex_unlock(&next_running_esi_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	return RUNNING_ESI;
}

void put_finish_esi(long esi_id){
	pthread_mutex_lock(&finished_list_mtx);
	queue_push_id(FINISHED_ESI_LIST, esi_id);
	pthread_mutex_unlock(&finished_list_mtx);
}

void borado_de_finish(){
	pthread_mutex_lock(&finished_list_mtx);
	log_debug(logger, "Deleting finished ESIs... Found %d (ids: %s)", queue_size(FINISHED_ESI_LIST), list_join(FINISHED_ESI_LIST->elements));
	while(!queue_is_empty(FINISHED_ESI_LIST)){
		long* esi_to_be_freed = queue_pop(FINISHED_ESI_LIST);
		free_esi(*esi_to_be_freed);
	}
	pthread_mutex_unlock(&finished_list_mtx);
}




