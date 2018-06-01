/*
 * connection_utils.c
 *
 */
#include "connection_utils.h"

void handshake(module_type module, void* handshake_message, int handshake_message_size) {
	int socket = connect_to(get_ip(module), get_port(module));
	set_socket(module, socket);
	if (socket < 0) {
		log_error(logger, "Could not connect to %s", get_module_name(module));
		exit_with_error();
	}

	int send_result = send(socket, handshake_message, handshake_message_size, 0);
	free(handshake_message);
	if (send_result < 0) {
		log_error(logger, "Could not send handshake");
		exit_with_error();
	}

	if (recv_message(socket) != CONNECTION_SUCCESS) {
		log_error(logger, "Rejected handshake");
		exit_with_error();
	}
}

char* get_module_name(module_type module) {
	switch(module) {
	case COORDINATOR: return "coordinator";
	case PLANIFIER: return "planifier";
	case INSTANCE: return "instance";
	case ISE: return "ise";
	default : return "unknown";
	}
}
