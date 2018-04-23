/*
 * planifier.h
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFIER_H_
#define PLANIFIER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <readline/readline.h> // Para usar readline
#include <pthread.h>
#include <commons/config.h>
#include "commons-sockets.h"

int server_port;
int server_max_connections;

int coordinator_socket;

//Global variables.
t_log * logger;

void configure_logger();

void load_configuration(char *config_file_path);

void listen_for_connections(int server_socket);

void exit_with_error(int socket, char* error_msg);

#endif /* PLANIFIER_H_ */
