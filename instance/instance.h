/*
 * instance.h
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <readline/readline.h> // Para usar readline
#include <commons/config.h>
#include "commons-sockets.h"
#include "types.h"
#include "response_codes.h"
#include "storage/entry-table.h"
#include "storage/replacement/replacement.h"
#include "commons/collections/dictionary.h"
#include <signal.h>
#include <string.h>
#include <pthread.h>

extern pthread_mutex_t atomic_operation;

typedef struct {
	int coordinator_port;
	char *coordinator_ip;
	char *instance_name;
	char *mount_path;
	int dump_interval;
	t_replacement_algorithm replacement_algorithm;
} t_instance_config;

int connect_to_coordinator(char *coordinator_ip, int coordinator_port);

t_instance_config* instance_config_create();

void instance_config_destroy(t_instance_config *instance_config);

pthread_t start_dump_interval();

//Global variables.
extern t_log * logger;
t_instance_config *instance_config = NULL;
t_entry_table* entries_table = NULL;
int coordinator_socket;

#endif /* INSTANCE_H_ */
