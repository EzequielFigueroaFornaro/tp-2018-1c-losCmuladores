#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>
#include "commons/collections/list.h"
#include "esi_structure.h"

esi* fifo_desbloquea_esis(int esis_id_liberadas);

void fifo_block_esi(t_list* BLOCKED_ESI_LIST, pthread_mutex_t* blocked_list_mtx,
					t_list* READY_ESI_LIST, pthread_mutex_t* ready_list_mtx,
					int* RUNNING_ESI, pthread_mutex_t*  running_esi_mtx,
					int* NEXT_RUNNING_ESI, pthread_mutex_t* next_running_esi_mtx,
					int esi_id);
void fifo_add_esi(t_list* ready_list, pthread_mutex_t* sem_ready_list, int esi);

void fifo_bloquea_esi(int esi);
void fifo_finish_esi(int esi);
void fifo_add_block_esi(int esi);

#endif
