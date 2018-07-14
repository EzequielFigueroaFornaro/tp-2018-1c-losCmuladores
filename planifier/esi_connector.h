/*
 * esi_connection_handler.h
 *
 *  Created on: 24 jun. 2018
 *      Author: utnso
 */

#ifndef ESI_CONNECTOR_H_
#define ESI_CONNECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>
#include "commons-sockets.h"
#include "types.h"
#include "response_codes.h"
#include "logging.h"
#include "semaphores.h"
#include "planifier_structures.h"
#include "orchestrator.h"

void esi_connection_handler(int socket);

bool send_esi_to_run(long esi_id);

bool wait_execution_result(long esi_id, int* result);

#endif /* ESI_CONNECTOR_H_ */
