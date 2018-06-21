#include "fifo.h"

void fifo_add_esi(long esi) {
	pthread_mutex_lock(&ready_list_mtx);
	log_debug(logger, "Adding ESI%ld to ready queue", esi);
	list_add(READY_ESI_LIST, &esi);
	pthread_mutex_unlock(&ready_list_mtx);
}

void fifo_block_esi(long block_esi_id){
	log_debug(logger, "Fifo blocking ESI%ld", block_esi_id);
	pthread_mutex_lock(&running_esi_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	pthread_mutex_lock(&blocked_list_mtx);
	pthread_mutex_lock(&ready_list_mtx);

	if(RUNNING_ESI == block_esi_id){
		fifo_replan();
	}else{
		bool equals_esi (long esi_id) {
				  return block_esi_id == esi_id;
			}
		log_debug(logger, "Removing ESI%ld from ready queue", block_esi_id);
		list_remove_by_condition(READY_ESI_LIST, (void*) equals_esi);
		//TODO, caso base
	}

	pthread_mutex_unlock(&ready_list_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	pthread_mutex_unlock(&next_running_esi_mtx);
	list_add(BLOCKED_ESI_LIST, &block_esi_id);
	pthread_mutex_unlock(&blocked_list_mtx);
}

void fifo_finish_esi() {
	pthread_mutex_lock(&ready_list_mtx);
	pthread_mutex_lock(&next_running_esi_mtx);
	fifo_replan();
	pthread_mutex_unlock(&ready_list_mtx);
	pthread_mutex_unlock(&next_running_esi_mtx);
}

void fifo_replan() {
	log_debug(logger, "Replaning...");

	long* esi_to_remove = list_get(READY_ESI_LIST, 0);
	log_debug(logger, "Removing ESI%ld from ready queue", *esi_to_remove);
	long* next_esi = list_remove(READY_ESI_LIST, 0);
	if (next_esi == NULL) {
		log_debug(logger, "No more ESIs to execute");
		NEXT_RUNNING_ESI = -1;
	} else {
		NEXT_RUNNING_ESI = *next_esi;
		log_debug(logger, "Next ESI to run is ESI%ld", NEXT_RUNNING_ESI);
	}
}

