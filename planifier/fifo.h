#ifndef FIFO_H_
#define FIFO_H_

#include "commons/collections/list.h"
#include "orchestrator.h"
#include "planifier.h"

esi* fifo_desbloquea_esis(int esis_id_liberadas);
void fifo_bloquea_esi(esi* esi);
void fifo_add_esi(esi* esi);
void fifo_finish_esi(esi* esi);
void fifo_add_block_esi(esi* esi);

#endif
