/*
 * list_cmd.c
 *
 *  Created on: 8 jun. 2018
 */
#include "list_cmd.h"

command_result list_cmd(command command) {
	char* resource = (char*) list_get(command.args, 0);
	command_result result = base_command_result(COMMAND_OK);
	result.content = blocked_queue_to_string(resource);
	return result;
}
