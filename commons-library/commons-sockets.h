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

typedef int module_type;

module_type COORDINATOR = 0;
module_type PLANIFIER = 1;
module_type INSTANCE = 2;
module_type ESI = 3;

typedef int message_type;

message_type MODULE_CONNECTED = 100;
message_type CONNECTION_SUCCESS = 101;

int start_server(int port, int max_connections);

int accept_connection(int server_socket);

int connect_to(char* ip, int port);

int send_connection_success(int socket);

int send_module_connected(int socket, module_type module_type);

char* get_client_address(int socket);
