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

//#define PORT 8080

int server_port;

//Global variables.
t_log * logger;


typedef struct  {
  int operation_id;
  int entries_size;
  //TODO y algo más que no entendí del enunciado.
} __attribute__((packed)) NewInstanceHeader;

#endif /* COORDINATOR_H_ */
