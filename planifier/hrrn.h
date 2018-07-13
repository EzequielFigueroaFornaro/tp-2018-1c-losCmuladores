/*
 * hrrn.h
 *
 *  Created on: 13 jul. 2018
 *      Author: utnso
 */

#ifndef HRRN_H_
#define HRRN_H_

#include "commons/collections/list.h"
#include "commons/string.h"
#include "planifier_structures.h"
#include "semaphores.h"
#include "logging.h"
#include "orchestrator.h"

void hrrn_add_esi(long esi);

void hrrn_block_esi(long block_esi_id);

void hrrn_replan();

void hrrn_finish_esi();

#endif /* HRRN_H_ */
