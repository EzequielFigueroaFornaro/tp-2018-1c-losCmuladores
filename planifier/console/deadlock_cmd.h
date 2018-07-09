/*
 * deadlock_cmd.h
 *
 *  Created on: 7 jul. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_DEADLOCK_CMD_H_
#define CONSOLE_DEADLOCK_CMD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

#include "command.h"
#include "../orchestrator.h"
#include "../planifier_structures.h"
#include "../semaphores.h"
#include "logging.h"

command_result deadlock_cmd(command command);

#endif /* CONSOLE_DEADLOCK_CMD_H_ */
