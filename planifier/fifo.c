#include "fifo.h"




void fifo_add_esi(t_list* ready_list, pthread_mutex_t* sem_ready_list, int esi){
	pthread_mutex_lock(&sem_ready_list);
	list_add(ready_list, esi);
	pthread_mutex_unlock(&sem_ready_list);
}

void fifo_block_esi(t_list* BLOCKED_ESI_LIST, pthread_mutex_t* blocked_list_mtx,
					t_list* READY_ESI_LIST, pthread_mutex_t* ready_list_mtx,
					int* RUNNING_ESI, pthread_mutex_t*  running_esi_mtx,
					int* NEXT_RUNNING_ESI, pthread_mutex_t* next_running_esi_mtx,
					int block_esi_id){
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	pthread_mutex_lock(&blocked_list_mtx);
	pthread_mutex_lock(&ready_list_mtx);

	if((&RUNNING_ESI)==block_esi_id){
		fifo_replan();
	}else{
		bool equals_esi (int esi_id) {
				  return block_esi_id == esi_id;
			}
		list_remove_by_condition(READY_ESI_LIST, equals_esi);
	}

	pthread_mutex_unlock(&ready_list_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	pthread_mutex_unlock(&next_running_esi_mtx);

	list_add(BLOCKED_ESI_LIST, block_esi_id);
	pthread_mutex_unlock(&blocked_list_mtx);
}

void fifo_replan(t_list* READY_ESI_LIST, int* NEXT_RUNNING_ESI){
	NEXT_RUNNING_ESI = list_remove(READY_ESI_LIST,0);
}



void fifo_finish_esi(esi* esi){

}








void put_on_list(t_list* list ,esi* esi, pthread_mutex_t sem_list){
	}

void remove_from_list(t_list* list, int index, pthread_mutex_t sem_list, esi* esi){
	pthread_mutex_lock(&sem_list);
	esi = list_remove(list, index);
	pthread_mutex_unlock(&sem_list);
}

