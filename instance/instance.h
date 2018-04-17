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

typedef struct  {
  int operation_id;
  int entries_size;
  //TODO y algo más que no entendí del enunciado.
} __attribute__((packed)) t_new_instance_header;

typedef struct {

} __attribute__((packed)) t_instance_configuration;

#endif /* INSTANCE_H_ */
