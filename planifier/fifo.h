#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/string.h"
#include "planifier_structures.h"
#include "semaphores.h"
#include "logging.h"

void fifo_add_esi(long esi);

void fifo_block_esi(long block_esi_id);

void fifo_replan();

void fifo_finish_esi();

#endif
