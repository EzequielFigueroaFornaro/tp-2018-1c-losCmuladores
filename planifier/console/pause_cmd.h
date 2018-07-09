/*
 * pause_cmd.h
 *
 *  Created on: 24 jun. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_PAUSE_CMD_H_
#define CONSOLE_PAUSE_CMD_H_

#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "command.h"
#include "console_log.h"
#include "../semaphores.h"
#include "../dispatcher.h"

command_result pause_cmd(command command);

#endif /* CONSOLE_PAUSE_CMD_H_ */
