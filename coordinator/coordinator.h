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
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/list.h"
#include "commons-sockets.h"
#include "response_codes.h"
#include "types.h"
#include "commons/txt.h"
#include <math.h>

#include "types.h"
#include "response_codes.h"
#include "handlers.h"

char* OPERATIONS_LOG_PATH;
//#define PORT 8080
int server_port;
int server_max_connections;
int delay;

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
pthread_mutex_t instances_list_mtx;
pthread_mutex_t keys_mtx;
pthread_mutex_t operations_log_file_mtx;

t_instance_configuration *instance_configuration;

typedef struct {
	pthread_t instance_thread;
	int socket_id;
	bool is_available;
	char* ip_port;
	char* name;
	int entries_in_use;
} t_instance;

t_instance *last_instance_selected;

typedef struct {
	pthread_t ise_thread;
	int socket_id;
	long id;
} __attribute__((packed)) t_ise;

t_instance* select_instance_to_send_by_distribution_strategy_and_operation(t_sentence* sentence);

int send_instance_configuration(int client_sock);

int receive_sentence_execution_request(int ise_socket, t_sentence** sentence);

int process_sentence(t_sentence* sentence, long ise_id);

void send_statement_result_to_ise(int socket, long ise_id, execution_result result);

void save_operation_log(t_sentence* sentence, long ise_id);

void exit_gracefully(int code);

int notify_sentence_and_ise_to_planifier(int operation_id, char* key, int ise_id);

#endif /* COORDINATOR_H_ */
