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
#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/list.h"
#include <pthread.h>
#include "commons-sockets.h"
#include "response_codes.h"
#include "types.h"
#include "commons/txt.h"
#include <signal.h>
#include <errno.h>
#include <pthread.h>

char* OPERATIONS_LOG_PATH = "operations.log";
//#define PORT 8080
int server_port;
int server_max_connections;

//planifier
int planifier_socket;

//Global variables.
t_log * logger;
t_list * instances_thread_list;
t_list * ise_thread_list;
t_config* config;

t_dictionary* keys_location; //TODO key -> t_instance

FILE* operations_log_file;

//Distribution
typedef enum { LSU, EL, KE } distributions;

distributions distribution;

//Semaphores
pthread_mutex_t instances_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t keys_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t operations_log_file_mtx = PTHREAD_MUTEX_INITIALIZER;

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
	char* ip_port;
	char* name;
} t_instance;

t_instance *last_instance_selected;

typedef struct {
	pthread_t ise_thread;
	int socket_id;
	int id;
} __attribute__((packed)) t_ise;

void _exit_with_error(int socket,char* error_msg, void * buffer);

#endif /* COORDINATOR_H_ */
