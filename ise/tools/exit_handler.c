/*
 * exit_handler.c
 */
#include "exit_handler.h"

void exit_gracefully(int code) {
	close(get_socket(COORDINATOR));
	close(get_socket(PLANIFIER));
	log_destroy(logger);
//	destroy_script(script);
	destroy_config();
	exit(code);
}

void exit_with_error() {
	exit_gracefully(EXIT_FAILURE);
}

