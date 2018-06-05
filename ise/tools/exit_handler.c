/*
 * exit_handler.c
 */
#include "exit_handler.h"

void close_connections();

void exit_gracefully(int code) {
	log_destroy(logger);
	destroy_script(script);
	close_connections();
	destroy_config();
	exit(code);
}

void exit_with_error() {
	exit_gracefully(EXIT_FAILURE);
}

void close_connections() {
	if (config_loaded) {
		close_connection(COORDINATOR);
		close_connection(PLANIFIER);
	}
}

