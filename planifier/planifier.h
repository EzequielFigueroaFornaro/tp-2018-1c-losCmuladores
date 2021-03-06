/*
 * planifier.h
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFIER_H_
#define PLANIFIER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Para close
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include "commons/collections/list.h"

#include "commons-sockets.h"
#include <types.h>
#include "logging.h"
#include <response_codes.h>

#include "orchestrator.h"
#include "planifier_structures.h"
#include "exit_handler.h"
#include "console/console.h"
#include "console/console_log.h"
#include "dispatcher.h"
#include "console/key_info.h"

int server_port;
int server_max_connections;
int coordinator_port;
char* coordinator_ip;
int coordinator_socket;

void load_configuration(char *config_file_path);

void connect_to_coordinator();

pthread_t start_console();

t_planifier_sentence* wait_for_statement_from_coordinator(int socket_id);

void try_to_block_resource(char* resource, long esi_id);

void send_execution_result_to_coordinator(execution_result result);

#endif /* PLANIFIER_H_ */
