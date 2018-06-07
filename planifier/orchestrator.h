#ifndef _ORCHESTRATOR_H_
#define _ORCHESTRATOR_H_


#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/collections/dictionary.h"
#include "esi_structure.h"
#include "fifo.h"

int ALGORITHM;


//pthread_mutex_t tiempo_cpu_sem = PTHREAD_MUTEX_INITIALIZER;
enum algorithm {
	FIFO = 1
};

void set_orchestrator(int algorithm);

void stop_and_block_esi(long esi_id);

void add_esi_bloqueada(long esi_id);

bool is_valid_esi(long esi_id)

#endif
