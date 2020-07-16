#ifndef sjf_desalojo_H_
#define sjf_desalojo_H_

#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/list.h"
#include "planifier_structures.h"
#include "semaphores.h"
#include "logging.h"
#include "orchestrator.h"

void sjf_desa_add_esi(long esi_id);
void sjf_desa_block_esi(long block_esi_id);
void sjf_desa_replan();
void sjf_desa_finish_esi();
void replan_for_new_esi();

#endif
