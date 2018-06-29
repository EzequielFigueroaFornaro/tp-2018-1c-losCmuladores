/*
 * dispatcher.h
 *
 *  Created on: 29 jun. 2018
 *      Author: utnso
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "logging.h"
#include "orchestrator.h"
#include "esi_connector.h"
#include "exit_handler.h"

void init_dispatcher();

void pause_dispatcher();

void resume_dispatcher();

#endif /* DISPATCHER_H_ */
