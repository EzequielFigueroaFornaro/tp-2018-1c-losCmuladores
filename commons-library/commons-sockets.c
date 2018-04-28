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

int send_module_connected(int socket, module_type module_type) {
	int message_size = sizeof(message_type) + sizeof(module_type);
	void *message_buffer = malloc(message_size);

	message_type message_type = MODULE_CONNECTED;

	memcpy(message_buffer, &message_type, sizeof(message_type));
	memcpy(message_buffer + sizeof(message_type), &module_type, sizeof(module_type));

	int result = send(socket, message_buffer, message_size, 0);

	free(message_buffer);

	return result;
}

int send_connection_success(int socket) {
	message_type message_type = CONNECTION_SUCCESS;
	return send(socket, &message_type, sizeof(message_type), 0);
}

char* get_client_address(int socket) {
	struct sockaddr_in client_address;
	socklen_t socket_lenght = sizeof(client_address);
	int err = getpeername(socket, (struct sockaddr *) &client_address, &socket_lenght);
	if (err == 0) {
		char ip[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &client_address.sin_addr, ip, sizeof ip);
		return string_from_format("%s:%d", ip, ntohs(client_address.sin_port));
	}
	return "unknown";
}
