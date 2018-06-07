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

pthread_mutex_t esi_map_mtx = PTHREAD_MUTEX_INITIALIZER;
t_dictionary * esi_map;

pthread_mutex_t running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
int RUNNING_ESI = -1;

pthread_mutex_t next_running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
int NEXT_RUNNING_ESI = 0;

pthread_mutex_t ready_list_mtx = PTHREAD_MUTEX_INITIALIZER;
t_list* READY_ESI_LIST;

pthread_mutex_t blocked_list_mtx = PTHREAD_MUTEX_INITIALIZER;
t_list* BLOCKED_ESI_LIST;

pthread_mutex_t finiched_list_mtx = PTHREAD_MUTEX_INITIALIZER;
t_list* FINISHED_ESI_LIST;

void set_orchestrator(int algorithm){
	ALGORITHM = algorithm;
	READY_ESI_LIST = list_create();
	BLOCKED_ESI_LIST = list_create();
	FINISHED_ESI_LIST = list_create();

};

/*aca se pueden tener un par de funciones globales para todas las esis
 * ejemplo, podemos tener un mapa de esis por id y el puntero a la esi, desde ahi podemos modificarlas mas rapidametne
 * usando el puntero en el mapa, en vez de ir ala lista correspondinte a tener que buscarla */


void add_esi(esi* esi){
	pthread_mutex_lock(&esi_map_mtx);
	dictionary_put(esi_map,string_key(esi->id), esi);
	pthread_mutex_unlock(&esi_map_mtx);
	switch(ALGORITHM) {
		case FIFO:
			fifo_add_esi(READY_ESI_LIST, &ready_list_mtx, (esi->id));
			break;
		default:
			fifo_add_esi(READY_ESI_LIST, &ready_list_mtx, (esi->id));
			break;
	}
}


void block_esi(int esi_id){
	modificar_estado(esi_id, BLOQUEADO);
	switch(ALGORITHM) {
			case FIFO:
				fifo_block_esi(BLOCKED_ESI_LIST, &blocked_list_mtx,
							   READY_ESI_LIST, &ready_list_mtx,
							   &RUNNING_ESI, &running_esi_mtx,
							   &NEXT_RUNNING_ESI, &next_running_esi_mtx,
							   esi_id);
				break;
			default:
				fifo_block_esi(BLOCKED_ESI_LIST, &blocked_list_mtx,
							   READY_ESI_LIST, &ready_list_mtx,
							   &RUNNING_ESI, &running_esi_mtx,
							   &NEXT_RUNNING_ESI, &next_running_esi_mtx,
							   esi_id);
				break;
		}
}

void modificar_estado(long esi_id, int nuevo_estado){
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_get(esi_map, string_key(esi_id));
	esi -> estado = nuevo_estado;
	pthread_mutex_unlock(&esi_map_mtx);
}

void unlock_esi(long esi_id){
	bool equals_esi (long esi) {
		  return esi_id == esi;
	}
	pthread_mutex_lock(&blocked_list_mtx);
	modificar_estado(esi_id, DESBLOQUEADO);
	list_remove_by_condition(BLOCKED_ESI_LIST,equals_esi);
	pthread_mutex_unlock(&blocked_list_mtx);
	switch(ALGORITHM) {
			case FIFO:
				fifo_add_esi(READY_ESI_LIST, &ready_list_mtx, esi_id);
				break;
			default:
				fifo_add_esi(READY_ESI_LIST, &ready_list_mtx, esi_id);
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
			list_remove_by_condition(BLOCKED_ESI_LIST, equals_esi);
			pthread_mutex_unlock(&blocked_list_mtx);
			break;
		case CORRIENDO:
			fifo_finish_esi(READY_ESI_LIST, &ready_list_mtx, NEXT_RUNNING_ESI, &next_running_esi_mtx);
			break;
		default:
			pthread_mutex_lock(&ready_list_mtx);
			list_remove_by_condition(READY_ESI_LIST, equals_esi);
			pthread_mutex_unlock(&ready_list_mtx);
			break;
	}
}

void free_esi(long esi_id){
	pthread_mutex_lock(&esi_map_mtx);
	esi* esi = dictionary_remove(esi_map, string_key(esi_id)); /*como mierda liberar el espacio del esi*/
	free(esi);
	pthread_mutex_unlock(&esi_map_mtx);
}

char* string_key(long key){
	return string_from_format("%l",key);
}

