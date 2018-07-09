#include "sjf.h"

void sjf_add_esi(long esi){
	pthread_mutex_lock(&ready_list_mtx);
	//TODO pasa el mapa de esis, hacer la cuenta , solo hay que pasar el que esta corriendo y el nuevo, si tiene mas prioridad
	// el nuvo lo pongo a correr sino lo pongo en la lista, en un add no hace falta ver todos los que estan en la cola
	list_add_id(READY_ESI_LIST, esi);
	pthread_mutex_unlock(&ready_list_mtx);
}

void sjf_block_esi(long block_esi_id){
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	pthread_mutex_lock(&blocked_list_mtx);
	pthread_mutex_lock(&ready_list_mtx);

	if(RUNNING_ESI == block_esi_id){
		sjf_replan();
	}else{
		bool equals_esi (long esi_id) {
				  return block_esi_id == esi_id;
		}
		list_remove_by_condition(READY_ESI_LIST, (void*) equals_esi);
	}

	pthread_mutex_unlock(&ready_list_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	pthread_mutex_unlock(&next_running_esi_mtx);
	list_add_id(BLOCKED_ESI_LIST, block_esi_id);
	pthread_mutex_unlock(&blocked_list_mtx);
}

void sjf_finish_esi(){
	sjf_replan();
}

void sjf_replan(){
	pthread_mutex_lock(&ready_list_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);

	// aca si me tengo que fijar de todos cual es el que voy a querer
	long* next_esi = list_remove(READY_ESI_LIST,0);
	NEXT_RUNNING_ESI = *next_esi;

	pthread_mutex_unlock(&ready_list_mtx);
	pthread_mutex_unlock(&next_running_esi_mtx);
}

