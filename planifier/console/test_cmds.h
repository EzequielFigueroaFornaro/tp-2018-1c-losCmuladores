/*
 * test_add_cmd.c
 *
 *  Created on: 21 jun. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_TEST_ADD_CMD_C_
#define CONSOLE_TEST_ADD_CMD_C_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

#include "command.h"
#include "../semaphores.h"
#include "../planifier_structures.h"
#include "../orchestrator.h"

command_result add_cmd(command command);

command_result list_esis_cmd(command command);

#endif /* CONSOLE_TEST_ADD_CMD_C_ */
