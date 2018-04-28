/*
 * esi.h
 *
 *  Created on: Apr 23, 2018
 *      Author: losCmuladores
 */

#ifndef SRC_ISE_H_
#define SRC_ISE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include "commons-sockets.h"
#include <signal.h>

int server_port;
int server_max_connections;

//coordinator
int coordinator_socket;

int planifier_socket;

int coordinator_port;
char* coordinator_ip;

int planifier_port;
char* planifier_ip;

//Global variables.
t_log * logger;

int handshake_planifier();

int handshake_coordinator();

void exit_gracefully(int code);

void load_configuration(char* config_file_path);

void configure_logger();

#endif /* SRC_ISE_H_ */
