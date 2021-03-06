/*
 * console_log.h
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#ifndef CONSOLE_CONSOLE_LOG_H_
#define CONSOLE_CONSOLE_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include "logging.h"

t_log* console_log;

void start_console_log();

void print_error(char* msg,...);

void print_and_log_error(char* msg,...);

void print_and_log(char* msg,...);

void log_on_both(char* msg,...);

void log_info_highlight(t_log* log, char* msg,...);

void log_info_important(t_log* log, char* msg,...);

#endif /* CONSOLE_CONSOLE_LOG_H_ */
