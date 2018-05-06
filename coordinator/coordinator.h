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
#include <string.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <readline/readline.h> // Para usar readline
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include "commons-sockets.h"
#include "types.h"
#include <signal.h>
#include <errno.h>

//#define PORT 8080
int server_port;
int server_max_connections;

//planifier
int planifier_socket;

//Global variables.
t_log * logger;
t_list * instances_thread_list;
t_list * ise_thread_list;

//Distribution
typedef enum { LSU, EL, KE } distributions;

distributions distribution;

pthread_mutex_t instances_mtx = PTHREAD_MUTEX_INITIALIZER;
t_instance last_selected;
//

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
	bool is_available;
} __attribute__((packed)) t_instance;

typedef struct {
	pthread_t ise_thread;
	int socket_id;
} __attribute__((packed)) t_ise;

void _exit_with_error(int socket,char* error_msg, void * buffer);

#endif /* COORDINATOR_H_ */
