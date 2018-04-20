#include "commons-sockets.h"
#include "commons/string.h"

int start_server(int port, int max_connections){
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	int enabled = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

	if (bind(server_socket, (void*) &server_address, sizeof(server_address)) != 0){
		perror("Error binding. Server not started.");
		return 1;
	}

	listen(server_socket, max_connections);

	return server_socket;
}

//TODO levantar threads por c/conexion aceptada.
int accept_connection(int server_socket){
	struct sockaddr_in cli_addr;
		socklen_t address_size;

		address_size = sizeof(cli_addr);
		int client_socket = accept(server_socket, (void*) &cli_addr, &address_size);

		if (client_socket == -1){
			return -1;
		}
		return client_socket;
}

int connect_to(char* ip, int port) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, string_itoa(port), &hints, &server_info);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (server_socket < 0) {
		perror("Error creating socket");
		return -1;
	}

	int connect_result = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);
	if (connect_result != 0) {
		return -1;
	}
	return server_socket;
}
