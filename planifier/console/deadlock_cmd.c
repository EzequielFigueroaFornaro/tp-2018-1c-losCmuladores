/*
 * deadlock_cmd.c
 *
 *  Created on: 7 jul. 2018
 *      Author: utnso
 */

#include "deadlock_cmd.h"

command_result deadlock_cmd(command command) {
	command_result result;
	result.code = COMMAND_OK;
	result.content = string_from_format("ESIs en deadlock: { %s }", list_join(buscar_deadlock()));
	return result;
}
