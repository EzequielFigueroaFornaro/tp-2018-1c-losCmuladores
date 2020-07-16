/*
 * console_log.c
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */
#include "console_log.h"

static char* RED = "\x1b[31m";
char* RESET = "\x1b[0m";
char* GREEN = "\x1b[1m\033[32m";
char* BLUE = "\x1b[1m\033[34m";

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
    va_end(arguments);
	log_error(console_log, message);
	printf("%s%s%s\n", RED, message, RESET);
	fflush(stdout);
	free(message);
}

void print_and_log(char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
    char* message = string_from_vformat(msg, arguments);
    va_end(arguments);
	log_info(console_log, message);
	printf("%s\n", message);
	fflush(stdout);
	free(message);
}

void log_on_both(char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
	char* message = string_from_vformat(msg, arguments);
	va_end(arguments);
	log_info(console_log, message);
	log_info(logger, message);
	free(message);
}

void log_info_with_colour(t_log* log, char* msg, char* colour) {
	char* with_colour = string_from_format("%s%s%s", colour, msg, RESET);
	log_info(log, with_colour);
	free(msg);
	free(with_colour);
}

void log_info_highlight(t_log* log, char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
	char* message = string_from_vformat(msg, arguments);
	va_end(arguments);
	log_info_with_colour(log, message, GREEN);
}

void log_info_important(t_log* log, char* msg,...) {
	va_list arguments;
	va_start(arguments, msg);
	char* message = string_from_vformat(msg, arguments);
	va_end(arguments);
	log_info_with_colour(log, message, BLUE);
}
