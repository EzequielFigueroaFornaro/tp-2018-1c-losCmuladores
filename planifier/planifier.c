/*
 ============================================================================
 Name        : planifier.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto planifier.
 ============================================================================
 */

#include "planifier.h"

long generate_esi_id();
void test_esi(int socket, long esi_id);
int test_executions_count = 0;

int main(int argc, char* argv[]) {
	configure_logger();
	load_configuration(argv[1]);

	connect_to_coordinator();

	int server_started = start_server(server_port, server_max_connections, (void *) esi_connection_handler, true, logger);
	if (server_started < 0) {
		log_error(logger, "Server not started");
	}

	pthread_t console_thread = start_console();
	pthread_join(console_thread, NULL);

	return EXIT_SUCCESS;
}

void configure_logger() {
	logger = log_create("planifier.log", "planifier", true, LOG_LEVEL_INFO);
}

void load_configuration(char *config_file_path) {
	log_info(logger, "Loading planifier configuration file...");
	t_config* config = config_create(config_file_path);

	server_port = config_get_int_value(config, "SERVER_PORT");
	server_max_connections = config_get_int_value(config,
			"MAX_ACCEPTED_CONNECTIONS");

	coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	coordinator_ip = string_duplicate(config_get_string_value(config, "COORDINATOR_IP"));

	config_destroy(config);

	log_info(logger, "Planifier configuration file loaded");
}

void connect_to_coordinator() {
	coordinator_socket = connect_to(coordinator_ip, coordinator_port);

	if (coordinator_socket < 0) {
		exit_with_error(coordinator_socket, "No se pudo conectar al coordinador");
	} else if (send_module_connected(coordinator_socket, PLANIFIER) < 0) {
		exit_with_error(coordinator_socket, "No se pudo enviar al confirmacion al coordinador");
	} else {
		message_type message_type;
		int message_type_result = recv(coordinator_socket, &message_type,
				sizeof(message_type), MSG_WAITALL);

		if (message_type_result < 0 || message_type != CONNECTION_SUCCESS) {
			exit_with_error(coordinator_socket, "Error al recibir confirmacion del coordinador");
		} else {
			log_info(logger, "Connexion con el coordinador establecida");
		}
	}
}

void send_coordinator_connection_completed(int coordinator_socket) {
	if (send_connection_success(coordinator_socket) < 0) {
		exit_with_error(coordinator_socket,
				"Error sending coordinator completed");
	} else {
		log_info(logger, "Coordinator connection completed");
	}
}

void esi_connection_handler(int socket) {
	message_type message_type;
	int result_message_type = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);

	if (result_message_type <= 0) {
		log_error(logger, "Error trying to receive message. Closing connection");
		close(socket);
		return;
	}

	if (message_type != MODULE_CONNECTED) {
		log_warning(logger, "Connection was received but the message type does not imply connection. Ignoring");
		close(socket);
		return;
	}

	module_type module_type;
	int result_module_type = recv(socket, &module_type, sizeof(module_type),
			MSG_WAITALL);
	if (result_module_type <= 0) {
		log_error(logger, "Error trying to receive module type. Closing connection");
		close(socket);
		return;
	}

	if (module_type == ISE) {
		long script_size;
		recv_long(socket, &script_size); // TODO Validar
		int message_size = sizeof(message_type) + sizeof(long);
		void* buffer = malloc(message_size);
		void* offset = buffer;
		concat_value(&offset, &CONNECTION_SUCCESS, sizeof(message_type));
		long esi_id = generate_esi_id();
		concat_value(&offset, &esi_id, sizeof(long));

		log_info(logger, "Sending connection success message to ESI %ld", esi_id);
		if (send(socket, buffer, message_size, 0) < 0) {
			log_error(logger, "Error sending \"connection success\" message to %s",	get_client_address(socket));
			close(socket);
			return;
		}
		free(buffer);
		log_info(logger, "ESI %ld connected! (from %s). Socket: %d, Script lines: %ld", esi_id, get_client_address(socket), socket, script_size);
		/**** PRUEBA *****/
		test_esi(socket, esi_id);
	} else {
		log_info(logger, "Ignoring connected client because it was not an ESI");
	}
}

/**** PRUEBA *****/
void test_esi(int socket, long esi_id) {
	send(socket, &ISE_EXECUTE, sizeof(message_type), 0);
	log_info(logger, "Waiting for ESI %ld notification", esi_id);
	while (test_executions_count < 3) {
		message_type msg;
		recv_msg(socket, &msg);
		if (msg != EXECUTION_RESULT) {
			log_error(logger,
					"Could not receive execution result message from ESI %ld",
					esi_id);
			return;
		}
		int execution_result;
		if (recv_int(socket, &execution_result) <= 0) {
			log_error(logger, "Could not receive execution result from ESI %ld",
					esi_id);
			return;
		}
		test_executions_count++;
		log_info(logger, "Executions count: %d", test_executions_count);
		log_info(logger, "Sending continue signal to ESI %ld", esi_id);
		send(socket, &ISE_EXECUTE, sizeof(message_type), 0);
	}
	log_info(logger, "Sending stop signal to ESI %ld", esi_id);
	send(socket, &ISE_STOP, sizeof(message_type), 0);
}

long generate_esi_id() {
	//TODO
	long esi_id = 42;
	log_info(logger, "Id %ld created for newly connected ESI", esi_id);
	return esi_id;
}

void listen_for_commands() {
	char *command;
	int is_exit_command;
	do {
		command = readline("Command: ");
		is_exit_command = string_equals_ignore_case(command, "EXIT");
		if (is_exit_command) {
			log_info(logger, "Exiting...");
		} else {
			log_info(logger, "Invalid command");
		}
		free(command);
	} while (!is_exit_command);
}

pthread_t start_console() {
	pthread_t console_thread;
	if (pthread_create(&console_thread, NULL, (void*) listen_for_commands, NULL) < 0) {
		exit_with_error(0, "Error starting console thread");
	};
	return console_thread;
}

void exit_gracefully(int return_nr) {
	log_destroy(logger);
	exit(return_nr);
}

void exit_with_error(int socket, char* error_msg) {
	log_error(logger, error_msg);
	close(socket);
	exit_gracefully(1);
}
