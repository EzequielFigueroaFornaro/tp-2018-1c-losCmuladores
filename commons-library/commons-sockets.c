#include "commons-sockets.h"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	int server_socket;
	void *(*connection_handler)(void *);
	t_log * logger;
} t_accept_params;

void accept_connections(t_accept_params *accept_params);
t_accept_params* build_accept_params(int server_socket, void *(*connection_handler)(void *), t_log *logger);

/* Starts a server in specified port and accepts incoming connections using threads.
 * Function _connection_handler must only receive a socket as argument
 */
int start_server(int port, int max_connections, void *(*_connection_handler)(void *), bool async, t_log *logger) {
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	int enabled = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

	if (bind(server_socket, (void*) &server_address, sizeof(server_address)) != 0){
		log_error(logger, "Error binding");
		close(server_socket);
		return -1;
	}

	listen(server_socket, max_connections);

	pthread_t listener_thread;
	t_accept_params* accept_params = build_accept_params(server_socket, _connection_handler, logger);
	if (pthread_create(&listener_thread, NULL, (void*) accept_connections, (void*) accept_params) < 0) {
		log_error(logger, "Could not create thread");
		return -1;
	}
	if (!async) {
		pthread_join(listener_thread, NULL);
	}
	return 0;
}

void accept_connections(t_accept_params *accept_params) {
	log_info(accept_params->logger, "Waiting for connections...");
	pthread_t thread;
	int client_sock;

	//TODO revisar si realmente necesito esto acá. Lo uso en la función que envía la configuración.
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	while ((client_sock = accept(accept_params->server_socket,
			(struct sockaddr *) &addr, &addrlen))) {
		if (client_sock < 0) {
			log_error(accept_params->logger, "Could not accept connection: %s",
					strerror(errno));
		}
		log_info(accept_params->logger, "Connection request received.");
		if (pthread_create(&thread, NULL, (void*) accept_params->connection_handler, (void*) client_sock) < 0) {
			log_error(accept_params->logger, "Could not create thread.");
			close(client_sock);
		} else {
			log_info(accept_params->logger, "Connection accepted !");
		}
	}
	pthread_join(thread, NULL);
	free(accept_params);
}

t_accept_params* build_accept_params(int server_socket, void *(*connection_handler)(void *), t_log *logger) {
	t_accept_params* accept_params = malloc(sizeof(t_accept_params));
	accept_params->server_socket = server_socket;
	accept_params->connection_handler = connection_handler;
	accept_params->logger = logger;
	return accept_params;
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
		freeaddrinfo(server_info);
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

int send_module_connected(int socket, module_type self) {
	int message_size = sizeof(message_type) + sizeof(self);
	void *message_buffer = malloc(message_size);

	message_type message_type = MODULE_CONNECTED;

	memcpy(message_buffer, &message_type, sizeof(message_type));
	memcpy(message_buffer + sizeof(message_type), &self, sizeof(self));

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

int recv_string(int socket, char** string) {
	int length;
	int length_result = recv(socket, &length, sizeof(length), MSG_WAITALL);
	if (length_result > 0) {
		*string = malloc(length);
		int result = recv(socket, *string, length, MSG_WAITALL);
		if (result < 0) {
			free(*string);
		}
		return result;
	} else {
		return length_result;
	}
}

int recv_sentence_operation(int socket, int *operation) {
	if (recv(socket, operation, sizeof(int), 0) <= 0 || !is_valid_operation(*operation)) {
		return -1;
	} else {
		return 1;
	}
}

message_type recv_message(int socket) {
	message_type message_type;
	int message_type_result = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);
	if (message_type_result <= 0) {
		return -1;
	}
	return message_type;
}
