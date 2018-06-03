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
	dictionary_put(esi_map, esi->id, esi);
	switch(ALGORITHM) {
		case FIFO:
			fifo_add_esi(esi);
			break;
		default:
			fifo_add_esi(esi);
			break;
	}
}


void add_esi_bloqueada(int esi_id){
	esi* ese_v = dictionary_get(esi_map, esi_id);

	bool equals_esi (esi* esi) {
		  return esi_id == (esi -> id);
	}
	list_remove_by_condition(BLOCKED_ESI_LIST,equals_esi);
	switch(ALGORITHM) {
			case FIFO:
				fifo_add_esi(ese_v);
				break;
			default:
				fifo_add_esi(ese_v);
				break;
		}
}

void finish(){

}



