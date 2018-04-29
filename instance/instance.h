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
#include "commons/collections/dictionary.h"

void connect_to_coordinator();

void _exit_with_error(int socket, char *error_msg, void *buffer);

//Global variables.
t_log * logger;
int coordinator_socket;
int coordinator_port;
char *coordinator_ip;


//Structs
typedef struct {
	int operation_id;
	long entries_size;
	long entries_quantity;
} __attribute__((packed)) t_instance_configuration;

t_instance_configuration *instance_configuration;

//La tabla de entradas guarda esta estructura.
typedef struct {
	char* key;
	void* entry_addr;
	int length;
} entry;

t_dictionary* entries_table;

#endif /* INSTANCE_H_ */
