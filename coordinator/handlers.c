/*
 * handlers.c
 *
 *  Created on: 13 jun. 2018
 *      Author: utnso
 */

#include "handlers.h"


void signal_handler(int sig){
    if (sig == SIGINT) {
    	log_info(logger,"Caught signal for Ctrl+C\n");
    	exit_gracefully(0);
    }
}

//TODO cerrar TODOS los sockets (planificador, el parametrizado, y el de todas las instancias conectadas)
void _exit_with_error(int socket, char* error_msg, void * buffer){
	if (buffer != NULL) {
		free(buffer);
	}
	log_error(logger, error_msg);
	close(socket);
	close(planifier_socket);
	exit_gracefully(1);
}


void connection_handler(int socket) {
	message_type message_type;
	int result_connected_message = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);
	if (result_connected_message <= 0 || message_type != MODULE_CONNECTED) {
		log_error(logger, "Error receiving connect message");
	} else {
		module_type module_type;
		int result_module = recv(socket, &module_type, sizeof(module_type), MSG_WAITALL);
		if (result_module <= 0) {
			log_error(logger, "Error receiving module type connected");
		} else if (module_type == INSTANCE) {
			instance_connection_handler(socket);
		} else if (module_type == PLANIFIER) {
			planifier_connection_handler(socket);
		} else if (module_type == ISE) {
			ise_connection_handler(socket);
		}
	}
}

void ise_connection_handler(int socket) {
	long ise_id;
	log_info(logger, "Socket del ESI: %d", socket);
	if (recv_long(socket, &ise_id) <= 0) {
		log_error(logger, "Could not receive ESI id");
		return;
	}

	if (send_connection_success(socket) < 0) {
		log_error(logger, "Error sending connection success to ESI %ld", ise_id);
		return;
	}

	log_info(logger, "ESI %ld connected", ise_id);

	log_info(logger, "Waiting for first statement...");
	t_sentence* sentence;
	int sentence_received;
	while ((sentence_received = receive_sentence_execution_request(socket, &sentence))) {
		if (sentence_received == MODULE_DISCONNECTED) {
			log_error(logger, "ESI %ld disconnected", ise_id);
			return;
		}
		if (sentence_received < 0) {
			log_error(logger, "Could not receive sentence from ESI %ld", ise_id);
			return;
		}

		log_info(logger, "Sentence received: %s", sentence_to_string(sentence));
		save_operation_log(sentence, ise_id);

		/* TEST: al primer GET devuelve key_blocked. En los siguientes GET y para cualquier
		  otra sentencia, devuelve OK al ESI (no hay comunicación con el planificador):    */

		// test_sentence_result(sentence, socket, ise_id);

		int execution_result = process_sentence(sentence, ise_id);
		send_statement_result_to_ise(socket, ise_id, execution_result);

		free(sentence);
	}
}

void instance_connection_handler(int socket) {
	char* instance_name;

	bool _is_existent_instance_connected(t_instance* instance){
		return strcmp(instance -> name, instance_name) == 0 && instance -> is_available == true;
	}

	//TODO ver qué info necesito, guardar en el struct de la instancia, y hacer free de todo lo necesario.
	if (send_connection_success(socket) < 0) {
		_exit_with_error(socket, "Error sending instance connection success", NULL);
	} else {

		int instance_name_result = recv_string(socket, &instance_name);

		if(instance_name_result <= 0){
			close(socket);
			log_error(logger, "Could not receive instance name");
			pthread_exit(pthread_self);//todo EXIT THREAD WITH ERROR.
			//_exit_with_error(socket, "Could not receive instance name", NULL);
		}

		if(list_any_satisfy(instances_thread_list, _is_existent_instance_connected)){
			close(socket);
			log_error(logger, "Another instance with same name is connected.");
			pthread_exit(pthread_self);//todo EXIT THREAD WITH ERROR.
						//_exit_with_error(socket, "Could not receive instance name", NULL);
		}

		int r = 1;
		int send_confirmation_result = send(socket, &r, sizeof(int), 0);

		int result = send_instance_configuration(socket);

		if(result != 0){
			log_error(logger, "Error while connecting instance.");
			return;
		}

		check_if_exists_or_create_new_instance(instance_name, socket);

		log_info(logger, "Instance connected");
	}
}

void planifier_connection_handler(int socket) {
	planifier_socket = socket;

	if (send_connection_success(socket) < 0) {
		_exit_with_error(socket, "Error sending planifier connection success", NULL);
	} else {
		log_info(logger, "Planifier connected");
	}
}

void handle_instance_disconnection(t_instance* instance){
	pthread_mutex_lock(&instances_list_mtx);
	instance -> is_available = false;
	pthread_mutex_unlock(&instances_list_mtx);
	last_instance_selected -> is_available = false; //TODO poner semaforo acá.
	log_info(logger, "Instance %s has been marked as UNAVAILABLE", instance -> ip_port);
}
