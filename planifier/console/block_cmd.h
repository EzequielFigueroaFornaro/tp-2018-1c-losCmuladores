/*
 * block_cmd.h
 *
 *  Created on: 9 jun. 2018
 */

#ifndef CONSOLE_BLOCK_CMD_H_
#define CONSOLE_BLOCK_CMD_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/queue.h>

#include "command.h"
#include "../orchestrator.h"
#include "../data.h"

command_result block_cmd(command command);

#endif /* CONSOLE_BLOCK_CMD_H_ */
