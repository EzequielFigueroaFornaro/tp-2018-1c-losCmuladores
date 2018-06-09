#include "sjf.h"

void sjf_add_esi(t_list* ready_list, pthread_mutex_t* sem_ready_list, long esi){
	pthread_mutex_lock(&sem_ready_list);
	//TODO pasa el mapa de esis, hacer la cuenta , solo hay que pasar el que esta corriendo y el nuevo, si tiene mas prioridad
	// el nuvo lo pongo a correr sino lo pongo en la lista, en un add no hace falta ver todos los que estan en la cola
	list_add(ready_list, esi);
	pthread_mutex_unlock(&sem_ready_list);
}

void sjf_block_esi(t_list* BLOCKED_ESI_LIST, pthread_mutex_t* blocked_list_mtx,
					t_list* READY_ESI_LIST, pthread_mutex_t* ready_list_mtx,
					long* RUNNING_ESI, pthread_mutex_t*  running_esi_mtx,
					long* NEXT_RUNNING_ESI, pthread_mutex_t* next_running_esi_mtx,
					long block_esi_id){
	pthread_mutex_lock(running_esi_mtx);
	pthread_mutex_lock(next_running_esi_mtx);
	pthread_mutex_lock(blocked_list_mtx);
	pthread_mutex_lock(ready_list_mtx);

	if((&RUNNING_ESI)==block_esi_id){
		sjf_replan(READY_ESI_LIST, NEXT_RUNNING_ESI);
	}else{
		bool equals_esi (int esi_id) {
				  return block_esi_id == esi_id;
		}
		list_remove_by_condition(READY_ESI_LIST, equals_esi);
	}

	pthread_mutex_unlock(ready_list_mtx);
	pthread_mutex_unlock(running_esi_mtx);
	pthread_mutex_unlock(next_running_esi_mtx);
	list_add(BLOCKED_ESI_LIST, block_esi_id);
	pthread_mutex_unlock(blocked_list_mtx);
}

void sjf_finish_esi(t_list* READY_ESI_LIST, pthread_mutex_t* ready_list_mtx,
					 long* NEXT_RUNNING_ESI, pthread_mutex_t* next_running_esi_mtx){
	pthread_mutex_lock(ready_list_mtx);
	pthread_mutex_lock(next_running_esi_mtx);
	sjf_replan(READY_ESI_LIST, NEXT_RUNNING_ESI);
	pthread_mutex_unlock(ready_list_mtx);
	pthread_mutex_unlock(next_running_esi_mtx);
}

void sjf_replan(t_list* READY_ESI_LIST, long* NEXT_RUNNING_ESI){
	// aca si me tengo que fijar de todos cual es el que voy a querer
	NEXT_RUNNING_ESI = list_remove(READY_ESI_LIST,0);
}
