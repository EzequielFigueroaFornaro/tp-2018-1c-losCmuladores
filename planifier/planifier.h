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
#include <commons/string.h>
#include <commons/log.h>
#include "commons-sockets.h"
#include <commons/collections/queue.h>
#include "commons/collections/list.h"
#include "orchestrator.h"
#include "esi_structure.h"

int server_port;
int server_max_connections;
int algorithm;
int id = 0;
int cpu_time = 0;

int coordinator_port;
char* coordinator_ip;

int coordinator_socket;

//Global variables.
t_log * logger;

t_dictionary * recursos_bloqueados;

pthread_mutex_t map_boqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ready_esi_sem_list = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t running_esi_sem_list = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blocked_esi_sem_list = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t finished_esi_sem_list = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t id_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cpu_time_mtx = PTHREAD_MUTEX_INITIALIZER;

void configure_logger();

void load_configuration(char *config_file_path);

void connect_to_coordinator();

pthread_t start_console();

void exit_with_error(int socket, char* error_msg);

void esi_connection_handler(int socket);

#endif /* PLANIFIER_H_ */
