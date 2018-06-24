/*
 * exit_handler.c
 *
 *  Created on: 20 jun. 2018
 *      Author: utnso
 */

#include "exit_handler.h"

bool console_closed = false;

void close_console(char* msg, int code) {
	if (console_closed) {
		return;
	}

	if (code == EXIT_SUCCESS) {
		print_and_log(msg);
	} else {
		print_and_log_error(msg);
	}
	close(0);
	destroy_command_config();
	log_destroy(console_log);
	console_closed = true;
}

void exit_gracefully(int return_nr) {
	if (esis_bloqueados_por_recurso != NULL) dictionary_destroy(esis_bloqueados_por_recurso);
	if (esi_map != NULL) {
		void close_esi_connection(char* esi_id, esi* esi) {
			close(esi->socket_id);
		}
		dictionary_iterator(esi_map, (void*) close_esi_connection);
		dictionary_destroy(esi_map);
	}
	if (READY_ESI_LIST != NULL) list_destroy(READY_ESI_LIST);
	if (BLOCKED_ESI_LIST != NULL) list_destroy(BLOCKED_ESI_LIST);
	if (FINISHED_ESI_LIST != NULL) queue_destroy(FINISHED_ESI_LIST);
	log_destroy(logger);
	close_console("Forced quit", 0);
	exit(return_nr);
}

void exit_with_error(int socket, char* error_msg) {
	log_error(logger, error_msg);
	print_error(error_msg);
	close(socket);
	exit_gracefully(1);
}

void quit_console(char* msg, int code) {
	close_console(msg, code);
	exit_gracefully(code);
}

void signal_handler(int sig) {
	if (sig == SIGINT) {
		log_info(logger, "Caught exit signal");
		exit_gracefully(0);
	}
}

void start_signal_listener() {
    signal(SIGINT, signal_handler);
}
