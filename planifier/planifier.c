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

	pthread_t listener_thread;
	if (pthread_create(&listener_thread, NULL, (void*) listen_for_connections,
			(void*) server_socket) < 0) {
		exit_with_error(server_socket, "Error in thread");
	};

	pthread_join(listener_thread, NULL);

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

	config_destroy(config);

	log_info(logger, "Planifier configuration file loaded");
}

void send_coordinator_connection_completed(int coordinator_socket) {
	int status = 1;
	int result = send(coordinator_socket, &status, sizeof(int), 0);
	if (result < 0) {
		exit_with_error(coordinator_socket, "Error sending coordinator completed");
	} else {
		log_info(logger, "Coordinator connection completed");
	}
}

void accept_connection_handler(int socket) {
	// falta logica para distinguir conexiones de coordinador con esi
	coordinator_socket = socket;

	int connection_type;
	int result = recv(socket, &connection_type, sizeof(connection_type), MSG_WAITALL);

	if (result < 0 || connection_type != 101) {
		exit_with_error(coordinator_socket, "Error with coordinator confirmation");
	} else {
		send_coordinator_connection_completed(coordinator_socket);
	}

}

void listen_for_connections(int server_socket) {
	log_info(logger, "Waiting for coordinator...");
	pthread_t instance_thread_id;

	int client_sock;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	while ((client_sock = accept(server_socket, (struct sockaddr *) &addr,
			&addrlen))) {
		log_info(logger, "Connection request received.");
		if (pthread_create(&instance_thread_id, NULL, (void*) accept_connection_handler, (void*) client_sock) < 0) {
			log_error(logger, "Could not create thread.");
		} else {
			log_info(logger, "Connection accepted !");
		}
	}

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
