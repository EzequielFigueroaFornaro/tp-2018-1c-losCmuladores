/*
 * config.h
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_COMMAND_CONFIG_H_
#define CONSOLE_COMMAND_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

typedef enum {
	EXIT,
	PAUSE,
	RESUME,
	BLOCK,
	UNBLOCK,
	LIST,
	STATUS,
	KILL,
	DEADLOCK,
	UNKNOWN,
	// TEST
	ADD,
	LIST_ESIS

} command_code;

typedef struct {
	command_code code;
	int args_count;
} command_config;

void load_commands();

command_config* get_command_config(char* command_str);

command_code get_command_code(char* command_str);

void destroy_command_config();

#endif /* CONSOLE_COMMAND_CONFIG_H_ */
