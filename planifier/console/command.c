/*
 * command.c
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */
#include "command.h"

void destroy_command(command command) {
	if (command.args != NULL) {
		list_destroy(command.args);
	}
}

bool is_exit(command command) {
	return EXIT == get_command_code(command.code_str);
}

bool command_exists(command command) {
	return UNKNOWN != get_command_code(command.code_str);
}

bool valid_args(command command) {
	return get_command_config(command.code_str)->args_count == list_size(command.args);
}

command parse_command(char* command_str) {
	char** splitted_command = string_split(command_str, " ");
	char** ptr = splitted_command;
	bool first_command_section = true;
	char* command_code_str;
	t_list* args = list_create();
	for (char* c = *ptr; c; c=*++ptr) {
		if (first_command_section) {
			command_code_str = c;
			first_command_section = false;
		} else {
			list_add(args, c);
		}
	}
	command command;
	command.code_str = command_code_str;
	command.args = args;
	return command;
}

command_result base_command_result(command_result_code code) {
	command_result result;
	result.code = code;
	return result;
}

command_result unblock_cmd(command command) {
	// TODO
	return base_command_result(COMMAND_OK);
}
