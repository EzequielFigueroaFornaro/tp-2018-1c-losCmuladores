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
#include <commons/log.h>
#include <commons/config.h>

int coordinator_port;
char *coordinator_ip;

//Global variables.
t_log * logger;

typedef struct {
	int operation_id;
	long entries_size;
	long entries_quantity;
} __attribute__((packed)) t_instance_configuration;

t_instance_configuration *instance_configuration = malloc(sizeof(t_instance_configuration));

#endif /* INSTANCE_H_ */
