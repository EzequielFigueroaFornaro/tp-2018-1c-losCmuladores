#ifndef _ORCHESTRATOR_H_
#define _ORCHESTRATOR_H_


#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "commons/collections/dictionary.h"
#include "commons/string.h"

#include "fifo.h"
#include "planifier_structures.h"
#include "semaphores.h"
#include "logging.h"

int ALGORITHM;

t_dictionary * esi_map;

//pthread_mutex_t tiempo_cpu_sem = PTHREAD_MUTEX_INITIALIZER;
enum algorithm {
	FIFO = 1
};

void add_esi(esi* esi);

void set_orchestrator(int algorithm);

void stop_and_block_esi(long esi_id);

void add_esi_bloqueada(long esi_id);

bool is_valid_esi(long esi_id);

char* string_key(long key);

long esi_se_va_a_ejecutar();

void borado_de_finish();

bool es_caso_base(long esi_id);

void block_esi(long esi_id);

void put_finish_esi(long esi_id);

void finish_esi(long esi_id);

void free_esi(long esi_id);

void volver_caso_base();

#endif
