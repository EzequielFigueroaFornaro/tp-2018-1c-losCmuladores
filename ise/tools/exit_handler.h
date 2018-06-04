/*
 * exit_handler.h
 */

#ifndef TOOLS_EXIT_HANDLER_H_
#define TOOLS_EXIT_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include "logging.h"
#include "script_handler.h"
#include "config.h"
#include "types.h"
#include "connection_utils.h"

void exit_gracefully(int code);

void exit_with_error();

#endif /* TOOLS_EXIT_HANDLER_H_ */
