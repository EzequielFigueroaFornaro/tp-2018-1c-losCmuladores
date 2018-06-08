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

t_log* console_log;

void start_console_log();

void print_error(char* msg,...);

void print_ok(char* msg,...);

#endif /* CONSOLE_CONSOLE_LOG_H_ */
