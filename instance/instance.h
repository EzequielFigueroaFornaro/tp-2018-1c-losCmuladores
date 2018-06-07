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
#include "storage/entry-table.h"
#include "commons/collections/dictionary.h"
#include <signal.h>
#include <string.h>

int connect_to_coordinator(char *coordinator_ip, int coordinator_port);

typedef struct {
	int coordinator_port;
	char *coordinator_ip;
	char *instance_name;
	char *mount_path;
	char *replacement_algorithm;
} t_instance_config;

//Global variables.
t_log * logger = NULL;
t_instance_config *instance_config = NULL;
int coordinator_socket;


t_entry_table* entries_table = NULL;

#endif /* INSTANCE_H_ */
