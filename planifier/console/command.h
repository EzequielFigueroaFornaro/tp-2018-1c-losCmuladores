/*
 * command.h
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_COMMAND_H_
#define CONSOLE_COMMAND_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#include "command_config.h"

typedef enum {
	COMMAND_OK = 1,
	COMMAND_ERROR = 2,
	INVALID_ARGS = 3,
	INVALID_COMMAND = 4
} command_result_code;

typedef struct {
	command_result_code code;
	char* content;
} command_result;

typedef struct {
	char* code_str;
	t_list* args;
} command;

void destroy_command(command command);

command parse_command(char* command_str);

bool command_exists(command command);

bool is_exit(command command);

bool valid_args(command command);

command_result base_command_result(command_result_code code);

#endif /* CONSOLE_COMMAND_H_ */
