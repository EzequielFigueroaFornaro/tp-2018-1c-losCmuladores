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
#include "tools/connections.h"

typedef int t_sentence_process_result;
t_sentence_process_result OK = 0;
t_sentence_process_result INVALID_SENTENCE = 1;
t_sentence_process_result KEY_NOT_FOUND = 2;

bool should_execute = false;

void init_execution_signals_receiver();
void wait_to_execute();
void execute_script();
t_sentence_process_result send_operation_to_coordinator(t_esi_operacion sentence);

void notify_sentence_processed_to_planifier(t_sentence_process_result result);
void notify_sentence_error(t_ise_sentence sentence);

void abort_on_sentence_error(t_ise_sentence sentence, t_ise_script* script);

void load_configuration(char* config_file_path);
void exit_gracefully(int code);

#endif /* ISE_H_ */
