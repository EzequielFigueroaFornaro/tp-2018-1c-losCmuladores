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

void add_esi(esi* esi){
	switch(ALGORITHM) {
		case FIFO: fifo_add_esi(esi);
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
void desbloquea_esis(esi* esi){
	switch(ALGORITHM) {
		case FIFO: fifo_desbloquea_esis(esi);
		default:
	}
}
