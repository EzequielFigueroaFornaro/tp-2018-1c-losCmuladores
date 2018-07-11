/*
 * handlers.c
 *
 *  Created on: 13 jun. 2018
 *      Author: utnso
 */

#include "handlers.h"

void check_if_exists_or_create_new_instance(char* instance_name, int socket){
	bool _is_same_instance_name(t_instance* instance){
		return strcmp(instance -> name, instance_name) == 0;
	}

	t_instance* instance;
	instance = (t_instance*) list_find(instances_thread_list, (void*)_is_same_instance_name);

	if(instance != NULL){
		instance -> is_available = true;
		instance -> instance_thread = pthread_self();
		instance -> socket_id = socket;
		instance -> ip_port = get_client_address(socket);
		//TODO ver qué pasa con la cantidad de entradas libres, y refactorizar esto.
	} else {
		instance = (t_instance*) malloc(sizeof(t_instance)); //TODO valgrind

		instance -> instance_thread = pthread_self();
		instance -> socket_id = socket;
		instance -> is_available = true;
		instance -> ip_port = get_client_address(socket);
		instance -> name = instance_name;
		instance -> entries_in_use = 0;

		list_add(instances_thread_list, instance);
	}
}

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
	if (socket >= 0) {
		close(socket);
	}
	close(planifier_socket);
	exit_gracefully(1);
}

void send_key_info_response(int socket, execution_result response_code, char* response) {
	if (!string_is_empty(response)) {
		int content_size = strlen(response) + 1;
		int response_size = sizeof(execution_result) + sizeof(int) + content_size;
		void* response_buffer = malloc(response_size);
		void* offset = response_buffer;
		concat_value(&offset, &response_code, sizeof(execution_result));
		concat_string(&offset, response, content_size);

		if (send(socket, response_buffer, response_size, 0) < 0) {
			log_error(logger, "[KeyInfoRequest] Error while trying to send instance name");
			return;
		}
	} else {
		if (send(socket, &response_code, sizeof(execution_result), 0) < 0) {
			log_error(logger, "[KeyInfoRequest] Error while trying to send key not found");
			return;
		}
	}
	log_info(logger,
			"[KeyInfoRequest] Response: description=%s, content=%s, sent to planifier",
			get_execution_result_description(response_code), response);
}

char* get_key_value_from_instance(char* key) {
	if (dictionary_has_key(keys_location, key)) {
		t_instance* instance = dictionary_get(keys_location, key);

		int key_size = strlen(key) + 1;
		int request_size = sizeof(message_type) + sizeof(int) + key_size;
		void* request = malloc(request_size);
		void* offset = request;
		concat_value(&offset, &GET_KEY_VALUE, sizeof(message_type));
		concat_string(&offset, key, key_size);

		if (send(instance->socket_id, request, request_size, 0) < 0) {
			log_error(logger,
					"[KeyInfoRequest] Could not send request to instance");
			return "";
		}

		int result;
		if (recv_int(instance->socket_id, &result) <= 0) {
			log_error(logger,
					"[KeyInfoRequest] Could not receive response code from instance");
			return "";
		}

		if (result == KEY_VALUE_FOUND) {
			char* value;
			if (recv_string(instance->socket_id, &value) <= 0) {
				log_error(logger,
						"[KeyInfoRequest] Could not receive value from instance");
				return "";
			}
			return value;
		}
	}
	return "";
}

void key_info_request_handler(int socket) {
	message_type request = recv_message(socket);

	if (request != GET_INSTANCE && request != CALCULATE_INSTANCE && request != GET_KEY_VALUE) {
		log_error(logger, "[KeyInfoRequest] Invalid request '%d'", request);
		close(socket);
		return;
	}

	char* key;
	if (recv_string(socket, &key) <= 0) {
		log_error(logger, "[KeyInfoRequest] Error while trying to receive key");
		close(socket);
		return;
	}

	if (request == GET_INSTANCE) {
		pthread_mutex_lock(&keys_mtx);
		if (dictionary_has_key(keys_location, key)) {
			t_instance* instance = dictionary_get(keys_location, key);
			pthread_mutex_unlock(&keys_mtx);
			send_key_info_response(socket, KEY_FOUND, instance->name);
		} else {
			pthread_mutex_unlock(&keys_mtx);
			send_key_info_response(socket, KEY_NOT_FOUND, "");
		}
	} else if (request == CALCULATE_INSTANCE) {
		t_sentence* sentence = malloc(sizeof(t_sentence));
		sentence->key = key;
		sentence->operation_id = SET_SENTENCE;
		t_instance* instance = select_instance_to_send_by_distribution_strategy_and_operation(sentence);
		free(sentence);

		if (instance != NULL) {
			send_key_info_response(socket, INSTANCE_AVAILABLE_FOR_KEY, instance->name);
		} else {
			send_key_info_response(socket, NO_INSTANCE_AVAILABLE_FOR_KEY, "");
		}
	} else if (request == GET_KEY_VALUE) {
		pthread_mutex_lock(&keys_mtx);
		char* value = get_key_value_from_instance(key);
		pthread_mutex_unlock(&keys_mtx);
		if (!string_is_empty(value)) {
			send_key_info_response(socket, KEY_VALUE_FOUND, value);
		} else {
			send_key_info_response(socket, KEY_VALUE_NOT_FOUND, "");
		}
	}
	if (recv_message(socket) != KEY_INFO_REQUEST_FINISHED) {
		log_info(logger, "Did not receive key info request finished message. Closing connection anyway");
	}
	close(socket);
	pthread_exit(pthread_self);
}


void connection_handler(int socket) {
	message_type message_type;
	int result_connected_message = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);
	if (result_connected_message <= 0) {
		log_error(logger, "Error receiving connect message");
		close(socket);
		return;
	}

	if (message_type == MODULE_CONNECTED) {
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
	} else if(message_type == KEY_INFO_REQUEST) {
		module_type module_type;
		int result_module = recv(socket, &module_type, sizeof(module_type), MSG_WAITALL);
		if (result_module <= 0 || module_type != PLANIFIER) {
			log_error(logger, "[KeyInfoRequest] Operation not permitted for module '%d'", module_type);
			close(socket);
			return;
		}
		key_info_request_handler(socket);
	} else {
		log_error(logger, "Received unknown connection message");
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

		if(list_any_satisfy(instances_thread_list, (void*)_is_existent_instance_connected)){
//			close(socket);
//			log_error(logger, "Another instance with same name is connected.");
//			pthread_exit(pthread_self);
//			//todo EXIT THREAD WITH ERROR.
//						//_exit_with_error(socket, "Could not receive instance name", NULL);
//		}



		int r = 1;
		int send_confirmation_result = send(socket, &r, sizeof(int), 0);

		int result = send_instance_configuration(socket, instance_name);

		if(result != 0){
			log_error(logger, "Error while connecting instance.");
			pthread_exit(pthread_self);
		}

		check_if_exists_or_create_new_instance(instance_name, socket);

		log_info(logger, "Instance connected: %s", instance_name);
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
