#ifndef _ORCHESTRATOR_H_
#define _ORCHESTRATOR_H_


#include <pthread.h>
#include "commons/collections/list.h"
#include "esi_structure.h"
#include "fifo.h"

int ALGORITHM;
t_list * READY_ESI_LIST;
t_list * RUNNING_ESI_LIST;
t_list * BLOCKED_ESI_LIST;
t_list * FINISHED_ESI_LIST;

//pthread_mutex_t tiempo_cpu_sem = PTHREAD_MUTEX_INITIALIZER;
int clock_cpu;

enum algorithm {
	FIFO = 1
};

void initialize_clock_and_sem();

void set_orchestrator(int algorithm, t_list* ready_esi_list, t_list* running_esi_list,
		t_list* blocked_esi_list, t_list* finished_esi_list);

//esi* get_esi_running();

void stop_and_block_esi(int esi_id);

void add_esi_bloqueada(int esi_id);

#endif
