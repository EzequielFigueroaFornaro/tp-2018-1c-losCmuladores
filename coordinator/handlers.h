/*
 * handlers.h
 *
 *  Created on: 13 jun. 2018
 *      Author: utnso
 */

#ifndef HANDLERS_H_
#define HANDLERS_H_

#include "coordinator.h"

void connection_handler(int socket);
void ise_connection_handler(int socket);
void instance_connection_handler(int socket);
void planifier_connection_handler(int socket);

//void handle_instance_disconnection(t_instance* instance);

void signal_handler(int sig);

void _exit_with_error(int socket, char* error_msg, void * buffer);

void check_if_exists_or_create_new_instance(char* instance_name, int socket);

#endif /* HANDLERS_H_ */
