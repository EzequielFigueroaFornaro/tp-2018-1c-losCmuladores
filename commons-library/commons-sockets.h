#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/collections/list.h>

int start_server(int port, int max_connections);

int accept_connection(int server_socket);

int connect_to(char* ip, int port);
