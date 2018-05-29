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



void set_orchestrator(char* algorithm,t_list* ready_esi_list,t_list* running_esi_list,
					  t_list* blocked_esi_list,t_list* finished_esi_list){
	ALGORITHM = algorithm;
	READY_ESI_LIST = ready_esi_list;
	RUNNING_ESI_LIST = running_esi_list;
	BLOCKED_ESI_LIST = blocked_esi_list;
	FINISHED_ESI_LIST = finished_esi_list;
};

/*aca se pueden tener un par de funciones globales para todas las esis
 * ejemplo, podemos tener un mapa de esis por id y el puntero a la esi, desde ahi podemos modificarlas mas rapidametne
 * usando el puntero en el mapa, en vez de ir ala lista correspondinte a tener que buscarla */




int tiempo_cpu_quantum = 0;
void ejecutar_esi(int esi){
	//voy al mapa y busco por el id, y por ejemplo calbio el stack del numero de sentencia en la que sta
	//o modificamos en base al tiempo de cpu  la prioridad en el hrrn
	pthread_mutex_trylock(&tiempo_cpu);
	int new_tiempo_cpu_quantum = tiempo_cpu_quantum ++;
	pthread_mutex_unlock(&tiempo_cpu);
}


void add_esi(esi* esi){
	switch(ALGORITHM) {
		case FIFO: fifo_add_esi(esi);
		default:
	}
}
void add_block_esi(esi* esi){
	switch(ALGORITHM) {
			case FIFO: fifo_add_block_esi(esi);
			default:
		}
}

void finish_esi(esi* esi){
	switch(ALGORITHM) {
		case FIFO: fifo_finish_esi(esi);
		default:
	}
}
void bloquea_esi(esi* esi){
	switch(ALGORITHM) {
		case FIFO: fifo_bloquea_esi(esi);
		default:
	}
}
esi* desbloquea_esis(int* esi){
	switch(ALGORITHM) {
		case FIFO: return fifo_desbloquea_esis(esi);
		default:
	}
}
void add_esi_bloqueada(esi_id){
	esi* esi = desbloquea_esis(esi_id);
	add_block_esi(esi);
}

