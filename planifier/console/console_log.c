/*
 * console_log.c
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */
#include "console_log.h"

static char* RED = "\x1b[31m";
static char* RESET = "\x1b[0m";

void start_console_log() {
	console_log = log_create("console.log", "planifier", 0, LOG_LEVEL_INFO);
}

void print_error(char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
	char* message = string_from_vformat(msg, arguments);
	printf("%s%s%s\n", RED, message, RESET);
	va_end(arguments);
	fflush(stdout);
	free(message);
}

void print_and_log_error(char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
    char* message = string_from_vformat(msg, arguments);
	log_error(console_log, message);
	printf("%s%s%s\n", RED, message, RESET);
	va_end(arguments);
	fflush(stdout);
	free(message);
}

void print_and_log(char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
    char* message = string_from_vformat(msg, arguments);
	log_info(console_log, message);
	printf("%s\n", message);
	va_end(arguments);
	fflush(stdout);
	free(message);
}
