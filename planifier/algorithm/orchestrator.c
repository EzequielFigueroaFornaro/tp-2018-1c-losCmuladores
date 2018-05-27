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

t_list* ready_esi_list,t_list* running_esi_list,t_list* blocked_esi_list,t_list* finished_esi_list

void add(){
	switch(ALGORITHM) {
		case FIFO:
		default:
	}
}
