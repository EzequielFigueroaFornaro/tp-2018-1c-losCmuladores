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



void set_orchestrator(int algorithm){
	ALGORITHM = algorithm;
	READY_ESI_LIST = list_create();
	BLOCKED_ESI_LIST = list_create();
	FINISHED_ESI_LIST = list_create();

};

/*aca se pueden tener un par de funciones globales para todas las esis
 * ejemplo, podemos tener un mapa de esis por id y el puntero a la esi, desde ahi podemos modificarlas mas rapidametne
 * usando el puntero en el mapa, en vez de ir ala lista correspondinte a tener que buscarla */


void ejecutar_esi(int esi){
	clock_cpu ++;
}

void add_esi(esi* esi){
	pthread_mutex_lock(&esi_map_mtx);
	dictionary_put(esi_map, esi->id, esi);
	pthread_mutex_unlock(&esi_map_mtx);
	switch(ALGORITHM) {
		case FIFO:
			fifo_add_esi(READY_ESI_LIST, ready_list_mtx, (esi->id));
			break;
		default:
			fifo_add_esi(READY_ESI_LIST, ready_list_mtx, (esi->id));
			break;
	}
}


void block_esi(int esi_id){
	switch(ALGORITHM) {
			case FIFO:
				fifo_block_esi(BLOCKED_ESI_LIST, blocked_list_mtx, READY_ESI_LIST, ready_list_mtx, RUNNING_ESI, running_esi_mtx, esi_id);
				break;
			default:
				fifo_block_esi(BLOCKED_ESI_LIST, blocked_list_mtx, READY_ESI_LIST, ready_list_mtx, RUNNING_ESI, running_esi_mtx, esi_id);
				break;
		}
}

void unlock_esi(int esi_id){
	bool equals_esi (int esi) {
		  return esi_id == esi;
	}
	pthread_mutex_lock(&blocked_list_mtx);
	list_remove_by_condition(BLOCKED_ESI_LIST,equals_esi);
	pthread_mutex_unlock(&blocked_list_mtx);
	switch(ALGORITHM) {
			case FIFO:
				fifo_add_esi(READY_ESI_LIST, ready_list_mtx, esi_id);
				break;
			default:
				fifo_add_esi(READY_ESI_LIST, ready_list_mtx, esi_id);
				break;
		}
}

void finish(){

}



