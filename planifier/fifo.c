#include "fifo.h"

void fifo_add_esi(long esi) {
	pthread_mutex_lock(&ready_list_mtx);
	log_debug(logger, "Adding ESI%ld to ready queue", esi);
	list_add_id(READY_ESI_LIST, esi);
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
		log_debug(logger, "Removing ESI%ld from ready queue", block_esi_id);
		list_remove_esi(READY_ESI_LIST, block_esi_id);
	}

	pthread_mutex_unlock(&ready_list_mtx);
	pthread_mutex_unlock(&running_esi_mtx);
	pthread_mutex_unlock(&next_running_esi_mtx);
	list_add_id(BLOCKED_ESI_LIST, block_esi_id);
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

	long* next_esi = list_remove(READY_ESI_LIST, 0);
	if (next_esi == NULL) {
		NEXT_RUNNING_ESI = 0;
		READY_ESI_LIST = list_create();
	} else {
		NEXT_RUNNING_ESI = *next_esi;
		log_debug(logger, "Next ESI to run is ESI%ld", NEXT_RUNNING_ESI);
	}
}

