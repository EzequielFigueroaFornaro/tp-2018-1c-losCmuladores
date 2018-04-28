/*
 ============================================================================
 Name        : ise.c
 Author      : losCmuladores
 ============================================================================
 */

#include "ise.h"

int main(int argc, char* argv[]) {
		configure_logger();
		log_info(logger, "Initializing...");
		load_configuration(argv[1]);

		int server_socket = start_server(server_port, server_max_connections);

		connect_to_coordinator();

		while(true){

		}
		return EXIT_SUCCESS;
}

void configure_logger() {
	logger = log_create("ise.log", "ise", 1, LOG_LEVEL_INFO);
}

void _exit_with_error(int socket,char* error_msg, void * buffer){
	if (buffer != NULL) {
		free(buffer);
	}
	log_error(logger, error_msg);
	close(socket);
	exit_gracefully(1);
}

int connect_to_coordinator() {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(coordinator_ip, coordinator_port, &hints, &server_info);

	int coordinator_socket = socket(server_info -> ai_family, server_info -> ai_socktype, server_info -> ai_protocol);

	int connect_status = connect(coordinator_socket, server_info -> ai_addr, server_info -> ai_addrlen);

	freeaddrinfo(server_info);

	if (connect_status < 0) {
		_exit_with_error(coordinator_socket, "No se pudo conectar al coordinador", NULL);
	} else if (send_module_connected(coordinator_socket, ISE) < 0) {
		_exit_with_error(coordinator_socket, "No se pudo enviar al confirmacion al coordinador", NULL);
	} else {
		message_type message_type;
		int message_type_result = recv(coordinator_socket, &message_type,
				sizeof(message_type), MSG_WAITALL);

		if (message_type_result < 0 || message_type != CONNECTION_SUCCESS) {
			_exit_with_error(coordinator_socket, "Error al recibir confirmacion del coordinador", NULL);
		} else {
			log_info(logger, "Connexion con el coordinador establecida");
		}
	}
	log_info(logger, "Conectado al coordinador!");
	return coordinator_socket;
}

void send_coordinator_connection(int coordinator_socket) {
	int connection_type = 401;
	int result = send(coordinator_socket, &connection_type, sizeof(int), 0);
	if (result < 0) {
		_exit_with_error(coordinator_socket, "Error trying to send coordinator connection confirmation. Code: " + result, NULL);
	} else {
		log_info(logger, "Connection confirmation with Coordinator sended");
	}
}

void recv_coordinator_connection(int coordinator_socket) {
	int status;
	int result = recv(coordinator_socket, &status, sizeof(int), MSG_WAITALL);
	log_info(logger, "RESULT CONNECT COORDINATOR %d", result);
	if (result < 0 || status != 1) {
		_exit_with_error(coordinator_socket, "Error trying to receive coordinator connection confirmation.", NULL);
	} else {
		log_info(logger, "Connection confirmation with Coordinator received");
	}
}

void exit_gracefully(int code) {
	log_destroy(logger);
	free(instance_configuration -> entries_quantity);
	free(instance_configuration -> entries_size);
	free(instance_configuration -> operation_id);
	free(instance_configuration);

	exit(code);
}

void load_configuration(char* config_file_path){
	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	coordinator_ip = config_get_string_value(config, "COORDINATOR_IP");
	coordinator_port = config_get_string_value(config, "COORDINATOR_PORT");

	log_info(logger, "OK.");
}
