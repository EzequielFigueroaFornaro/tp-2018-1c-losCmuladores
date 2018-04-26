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

int main(int argc, char* argv[]) {
	configure_logger();
	load_configuration(argv[1]);

	int server_socket = start_server(server_port, server_max_connections);

//	connect_to_coordinator();

	pthread_t listener_thread;
	if (pthread_create(&listener_thread, NULL, (void*) listen_for_esi_connections,
			(void*) server_socket) < 0) {
		exit_with_error(server_socket, "Error in thread");
	};

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

	if (result_message_type < 0 /*|| message_type != MODULE_CONNECTED*/) {
		exit_with_error(socket,
				"Error with module connection confirmation");
	}

	module_type module_type;
	int result_module_type = recv(socket, &module_type, sizeof(module_type),
			MSG_WAITALL);
	if (result_module_type < 0) {
		exit_with_error(socket, "Error receiving module type");
	}

	if (module_type == ESI) {
		if (send_connection_success(socket) < 0) {
			exit_with_error(socket,
					string_from_format(
							"Error sending \"connection success\" message to %s",
							get_client_address(socket)));
		}
		log_info(logger, "ESI connected! (from %s)", get_client_address(socket));
	} else {
		log_info(logger, "Ignoring connected client because it was not an ESI");
	}
}

void listen_for_esi_connections(int server_socket) {
	log_info(logger, "Waiting for esis...");
	pthread_t instance_thread_id;

	int client_sock;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	while ((client_sock = accept(server_socket, (struct sockaddr *) &addr,
			&addrlen))) {
		log_info(logger, "Connection request received.");
		if (pthread_create(&instance_thread_id, NULL,
				(void*) esi_connection_handler, (void*) client_sock) < 0) {
			log_error(logger, "Could not create thread.");
		} else {
			log_info(logger, "Connection accepted !");
		}
	}

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
