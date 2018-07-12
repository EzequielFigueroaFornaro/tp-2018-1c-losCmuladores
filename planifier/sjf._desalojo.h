#ifndef sjf_H_
#define sjf_H_

#include <pthread.h>
#include "commons/collections/list.h"
#include "planifier_structures.h"
#include "semaphores.h"

void sjf_desa_add_esi(long esi);
void sjf_desa_block_esi(long block_esi_id);
void sjf_desa_replan();
void sjf_desa_finish_esi();

#endif
