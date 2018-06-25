/*
 * esi_connection_handler.h
 *
 *  Created on: 24 jun. 2018
 *      Author: utnso
 */

#ifndef ESI_CONNECTION_HANDLER_H_
#define ESI_CONNECTION_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>
#include "commons-sockets.h"
#include <types.h>
#include "logging.h"
#include "semaphores.h"
#include "orchestrator.h"

void esi_connection_handler(int socket);

#endif /* ESI_CONNECTION_HANDLER_H_ */
