/*
 ============================================================================
 Name        : coordinator.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto coordinator
 ============================================================================
 */

#include "coordinator.h"

void configure_logger() {
	logger = log_create("coordinator.log", "coordinator", true, LOG_LEVEL_INFO);
}

//TODO llevar a commons
int start_server(){
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(PORT);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	int enabled = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

	if (bind(server_socket, (void*) &server_address, sizeof(server_address)) != 0){
		perror("Error binding. Server not started.");
		return 1;
	}

	log_info(logger, "Listening on port %d", PORT);
	listen(server_socket, 100);

	return server_socket;
}

//TODO llevar a commons
int accept_connection(int server_socket){
	struct sockaddr_in cli_addr;
	socklen_t address_size;

	address_size = sizeof(cli_addr);
	int client_socket = accept(server_socket, (void*) &cli_addr, &address_size);

	if (client_socket == -1){
		log_error(logger, "Could not accept connection.");
		return -1; //TODO acá habría que cerrar socket con algúna llamada a algo de commons y matar el proceso.
	}

	return client_socket;
}

int main(void) {
	puts("!!!Hello World del Coordinador!!!"); /* prints !!!Hello World!!! */
	configure_logger();
	int server_socket = start_server();
	int client_socket = accept_connection(server_socket);
	log_info(logger, "Acepte conexión. CLient: %d", client_socket);
	close(server_socket);
	return EXIT_SUCCESS;
}
