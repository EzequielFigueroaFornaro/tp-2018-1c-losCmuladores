#ifndef _ORCHESTRATOR_H_
#define _ORCHESTRATOR_H_


#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/collections/dictionary.h"
#include "esi_structure.h"
#include "fifo.h"

int ALGORITHM;

pthread_mutex_t esi_map_mtx = PTHREAD_MUTEX_INITIALIZER;
t_dictionary * esi_map;

pthread_mutex_t running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
int RUNNING_ESI=-1;

pthread_mutex_t next_running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
int NEXT_RUNNING_ESI=0;

pthread_mutex_t ready_list_mtx = PTHREAD_MUTEX_INITIALIZER;
t_list * READY_ESI_LIST;

pthread_mutex_t blocked_list_mtx = PTHREAD_MUTEX_INITIALIZER;
t_list * BLOCKED_ESI_LIST;

pthread_mutex_t finiched_list_mtx = PTHREAD_MUTEX_INITIALIZER;
t_list * FINISHED_ESI_LIST;


//pthread_mutex_t tiempo_cpu_sem = PTHREAD_MUTEX_INITIALIZER;
int clock_cpu;

enum algorithm {
	FIFO = 1
};

void set_orchestrator(int algorithm);

//esi* get_esi_running();

void stop_and_block_esi(int esi_id);

void add_esi_bloqueada(int esi_id);

#endif
