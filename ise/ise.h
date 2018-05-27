/*
 * esi.h
 *
 *  Created on: Apr 23, 2018
 *      Author: losCmuladores
 */
#ifndef ISE_H_
#define ISE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <commons/log.h>
#include "tools/script_handler.h"
#include "tools/logging.h"
#include "tools/config.h"
#include "commons-sockets.h"
#include "types.h"

typedef int t_sentence_process_result;
t_sentence_process_result OK = 0;
t_sentence_process_result INVALID_SENTENCE = 1;
t_sentence_process_result KEY_NOT_FOUND = 2;

char* my_id = "ESI42";

int coordinator_socket;
int planifier_socket;

void connect_to_planifier();
void connect_to_coordinator();
void wait_to_execute();
void execute_script();
t_sentence_process_result send_sentence_to_coordinator(t_esi_operacion operation);

void notify_planifier();
void notify_error();

void exit_gracefully(int code);

#endif /* ISE_H_ */
