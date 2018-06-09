/*
 * list_cmd.h
 *
 *  Created on: 8 jun. 2018
 */

#ifndef CONSOLE_COMMANDS_LIST_CMD_H_
#define CONSOLE_COMMANDS_LIST_CMD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/queue.h>

#include "command.h"
#include "../data.h"

command_result list_cmd(command command);

#endif /* CONSOLE_COMMANDS_LIST_CMD_H_ */
