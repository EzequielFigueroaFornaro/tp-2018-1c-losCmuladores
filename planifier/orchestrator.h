#ifndef _ORCHESTRATOR_H_
#define _ORCHESTRATOR_H_


#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/collections/dictionary.h"
#include "esi_structure.h"
#include "fifo.h"

int ALGORITHM;


//pthread_mutex_t tiempo_cpu_sem = PTHREAD_MUTEX_INITIALIZER;
int clock_cpu;

enum algorithm {
	FIFO = 1
};

void set_orchestrator(int algorithm);

void stop_and_block_esi(int esi_id);

void add_esi_bloqueada(int esi_id);

#endif
