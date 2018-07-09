/*
 * exit_handler.h
 *
 *  Created on: 20 jun. 2018
 *      Author: utnso
 */

#ifndef EXIT_HANDLER_H_
#define EXIT_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

#include "logging.h"

#include "planifier_structures.h"
#include "console/console_log.h"
#include "console/command_config.h"
#include "orchestrator.h"

void exit_gracefully(int return_nr);

void exit_with_error(int socket, char* error_msg);

void start_signal_listener();

void quit_console(char* msg, int code);

#endif /* EXIT_HANDLER_H_ */
