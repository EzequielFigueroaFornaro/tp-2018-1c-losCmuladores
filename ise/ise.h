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
#include <pthread.h>
#include <commons/log.h>

#include "commons-sockets.h"
#include "types.h"
#include "response_codes.h"

#include "tools/script_handler.h"
#include "tools/logging.h"
#include "tools/config.h"
#include "tools/exit_handler.h"
#include "tools/connection_utils.h"

int my_id = 42;

int self_module_type = ISE;

void connect_to_planifier();
void connect_to_coordinator();
void wait_to_execute();
void execute_script();
execution_result send_sentence_to_coordinator(t_esi_operacion operation);
void handle_execution_result(execution_result result);
void notify_planifier(execution_result result);

#endif /* ISE_H_ */
