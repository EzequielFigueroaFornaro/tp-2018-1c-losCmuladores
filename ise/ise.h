/*
 * ise.h
 *
 *  Created on: Apr 23, 2018
 *      Author: losCmuladores
 */

#ifndef SRC_ISE_H_
#define SRC_ISE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <readline/readline.h> // Para usar readline
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
char *coordinator_port;
char *coordinator_ip;


//Global variables.
t_log * logger;
t_list * instances_thread_list;

typedef struct {
	int operation_id;
	long entries_size;
	long entries_quantity;
}__attribute((packed)) t_instance_configuration;

t_instance_configuration *instance_configuration;

#endif /* SRC_ISE_H_ */
