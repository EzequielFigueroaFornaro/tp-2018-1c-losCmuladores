/*
 * exit_handler.c
 */
#include "exit_handler.h"

void close_connections();
void signal_handler(int sig);

void exit_gracefully(int code) {
	destroy_script(script);
	close_connections();
	destroy_config();
	log_info(logger, "Bye!");
	log_destroy(logger);
	exit(code);
}

void exit_with_error() {
	exit_gracefully(EXIT_FAILURE);
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
    	log_info(logger,"Caught exit signal");
    	exit_gracefully(0);
    }
}

void start_signal_listener() {
    signal(SIGINT, signal_handler);
}

void close_connections() {
	if (config_loaded) {
		close_connection(COORDINATOR);
		close_connection(PLANIFIER);
	}
}

