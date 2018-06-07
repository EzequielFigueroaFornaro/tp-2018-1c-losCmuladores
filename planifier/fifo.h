#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>
#include "commons/collections/list.h"
#include "esi_structure.h"

void fifo_add_esi(t_list* ready_list, pthread_mutex_t* sem_ready_list, int esi);

void fifo_block_esi(t_list* BLOCKED_ESI_LIST, pthread_mutex_t* blocked_list_mtx,
					t_list* READY_ESI_LIST, pthread_mutex_t* ready_list_mtx,
					long* RUNNING_ESI, pthread_mutex_t*  running_esi_mtx,
					long* NEXT_RUNNING_ESI, pthread_mutex_t* next_running_esi_mtx,
					long block_esi_id);

void fifo_replan(t_list* READY_ESI_LIST, long* NEXT_RUNNING_ESI);

#endif
