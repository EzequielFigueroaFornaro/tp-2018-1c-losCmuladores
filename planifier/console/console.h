/*
 * console.h
 *
 *  Created on: 7 jun. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <readline/readline.h>

#include "command.h"
#include "command_config.h"
#include "console_log.h"
#include "../exit_handler.h"

// Commands
#include "list_cmd.h"
#include "block_cmd.h"
#include "unblock_cmd.h"
#include "pause_cmd.h"

#include "test_cmds.h"

pthread_t start_console();

#endif /* CONSOLE_H_ */
