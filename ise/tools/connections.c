#include "connections.h"
#include "commons-sockets.h"

int coordinator_socket;
int planifier_socket;

int handshake(char* ip, int port, module_type module);

bool init_connections() {
	coordinator_socket = handshake(coordinator_ip, coordinator_port, COORDINATOR);
	planifier_socket = handshake(planifier_ip, planifier_port, PLANIFIER);

	return !(coordinator_socket < 0 || planifier_socket < 0);
}

// TODO [LU] enviar y recibir id de esi
int handshake(char* ip, int port, module_type module) {
	char* module_name = get_module_name_by_type(module);
	log_info(logger, "Starting handshake process with %s...", module_name);
	int connect_result = connect_to(ip, port);
	if (connect_result < 0) {
		log_error(logger, "Could not connect to %s on %s:%d", module_name, ip,
				port);
		return -1;
	}

	int socket = connect_result;
	if (send_module_connected(socket, ISE) < 0) {
		log_error(logger, "Could not send message to %s on %s:%d", module_name,
				ip, port);
		close(socket);
		return -1;
	}

	message_type message_type;
	int message_type_result = recv(socket, &message_type, sizeof(message_type),
			MSG_WAITALL);

	if (message_type_result <= 0 || message_type != CONNECTION_SUCCESS) {
		log_error(logger,
				"Expected CONNECTION_SUCCESS (%d) from %s on %s:%d, actual: %d",
				module_name, CONNECTION_SUCCESS, ip, port, message_type);
		close(socket);
		return -1;
	}
	log_info(logger, "Connected to %s!", module_name);
	return connect_result;
}

