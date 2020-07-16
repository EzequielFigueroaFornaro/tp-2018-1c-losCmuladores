/*
 * status_cmd.h
 *
 *  Created on: 7 jul. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_STATUS_CMD_H_
#define CONSOLE_STATUS_CMD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "key_info.h"
#include "command.h"
#include "../orchestrator.h"
#include "../planifier_structures.h"
#include "logging.h"

command_result status_cmd(command command);

#endif /* CONSOLE_STATUS_CMD_H_ */
