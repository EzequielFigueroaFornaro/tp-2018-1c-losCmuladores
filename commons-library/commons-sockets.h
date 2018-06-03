#ifndef COMMONS_SOCKET_H_
#define COMMONS_SOCKET_H_

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

typedef int module_type;

module_type COORDINATOR = 0;
module_type PLANIFIER = 1;
module_type INSTANCE = 2;
module_type ISE = 3;

typedef int message_type;

message_type MODULE_CONNECTED = 100;
message_type CONNECTION_SUCCESS = 101;

int start_server(int port, int max_connections, void *(*_connection_handler)(void *), bool async, t_log *logger);

int connect_to(char* ip, int port);

int send_connection_success(int socket);

int send_module_connected(int socket, module_type module_type);

char* get_client_address(int socket);

int recv_string(int socket, char** string);

int recv_sentence_operation(int socket, int *operation);

#endif
