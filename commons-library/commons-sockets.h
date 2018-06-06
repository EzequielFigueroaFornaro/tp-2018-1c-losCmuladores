#ifndef COMMONS_SOCKETS_H_
#define COMMONS_SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include "types.h"

extern int MODULE_DISCONNECTED;

int start_server(int port, int max_connections, void *(*_connection_handler)(void *), bool async, t_log *logger);

int connect_to(char* ip, int port);

int send_connection_success(int socket);

int send_module_connected(int socket, module_type module_type);

char* get_client_address(int socket);

int recv_string(int socket, char** string);

int recv_int(int socket, int* value);

int recv_long(int socket, long* id);

int recv_sentence_operation(int socket, int *operation);

#endif /* COMMONS_SOCKETS_H_ */
