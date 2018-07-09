/*
 * coordinator_connector.h
 *
 *  Created on: 8 jul. 2018
 *      Author: utnso
 */

#ifndef KEY_INFO_H_
#define KEY_INFO_H_

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

typedef enum {
	value,
	instance,
	calculated_instance
} key_param;

char* get_key_param(key_param param, char* key);

void set_coordinator_socket(int socket);

#endif /* KEY_INFO_H_ */
