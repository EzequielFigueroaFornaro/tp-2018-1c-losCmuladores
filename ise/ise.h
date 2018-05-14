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
#include <commons/config.h>
#include <pthread.h>
#include <commons/log.h>
#include "tools/script_handler.h"
#include "tools/logging.h"
#include "commons-sockets.h"

typedef int t_sentence_process_result;
t_sentence_process_result OK = 0;
t_sentence_process_result INVALID_SENTENCE = 1;
t_sentence_process_result KEY_NOT_FOUND = 2;

char* my_id;
message_type execution_signal;

int coordinator_socket;
int planifier_socket;

int coordinator_port;
char* coordinator_ip;

int planifier_port;
char* planifier_ip;

void connect_to_planifier();
void connect_to_coordinator();
void wait_to_execute();
void execute_script();
t_sentence_process_result send_operation(t_esi_operacion operation);

message_type notify();
void notify_error();

void load_configuration(char* config_file_path);
void exit_gracefully(int code);

#endif /* ISE_H_ */
