#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>
#include "commons/collections/list.h"
#include "esi_structure.h"

esi* fifo_desbloquea_esis(int esis_id_liberadas);
void fifo_bloquea_esi(esi* esi);
void fifo_add_esi(esi* esi);
void fifo_finish_esi(esi* esi);
void fifo_add_block_esi(esi* esi);

#endif
