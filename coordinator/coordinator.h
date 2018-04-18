/*
 * coordinator.h
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#ifndef COORDINATOR_H_
#define COORDINATOR_H_

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

//#define PORT 8080
int server_port;

//Global variables.
t_log * logger;

typedef struct  {
  int operation_id;
  //TODO lo que sea necesario.
} __attribute__((packed)) t_new_instance_header;

typedef struct {
	int operation_id;
	long entries_size;
	long entries_quantity;
}__attribute((packed)) t_instance_configuration;

t_instance_configuration *instance_configuration;

typedef struct {
	pthread_t instance_thread;
	int socket_id;
} __attribute__((packed)) t_instance;

#endif /* COORDINATOR_H_ */
