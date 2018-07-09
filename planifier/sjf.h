#ifndef sjf_H_
#define sjf_H_

#include <pthread.h>
#include "commons/collections/list.h"
#include "planifier_structures.h"
#include "semaphores.h"

void sjf_add_esi(long esi);

void sjf_block_esi(long block_esi_id);

void sjf_replan();

void sjf_finish_esi();

#endif
