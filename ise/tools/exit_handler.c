/*
 * exit_handler.c
 */
#include "exit_handler.h"

void exit_gracefully(int code) {
	close_connection(COORDINATOR);
	close_connection(PLANIFIER);
	log_destroy(logger);
	destroy_script(script);
	destroy_config();
	exit(code);
}

void exit_with_error() {
	exit_gracefully(EXIT_FAILURE);
}

