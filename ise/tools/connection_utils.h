/*
 * connection_utils.h
 */

#ifndef TOOLS_CONNECTION_UTILS_H_
#define TOOLS_CONNECTION_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "commons-sockets.h"
#include "types.h"

#include "exit_handler.h"
#include "logging.h"

void handshake(module_type module, void* handshake_message, int handshake_message_size);

char* get_module_name(module_type module);

char* get_ip(module_type module);

int get_port(module_type module);

int get_socket(module_type module);

void set_socket(module_type module, int socket);

void close_connection(module_type module);

#endif /* TOOLS_CONNECTION_UTILS_H_ */
