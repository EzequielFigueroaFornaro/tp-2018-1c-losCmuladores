/*
 ============================================================================
 Name        : planifier.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto planifier	.
 ============================================================================
 */

#include "planifier.h"

void load_configuration(char *config_file_path) {
	log_info(logger, "Loading planifier configuration file...");
	t_config* config = config_create(config_file_path);

	t_dictionary* algorithms = dictionary_create();
	dictionary_put(algorithms, "FIFO", string_from_format("%d", FIFO));

	char* algorithm_code = config_get_string_value(config, "ALGORITHM");
	algorithm = atoi(dictionary_get(algorithms, algorithm_code));

	server_port = config_get_int_value(config, "SERVER_PORT");
	server_max_connections = config_get_int_value(config,
			"MAX_ACCEPTED_CONNECTIONS");

	coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	coordinator_ip = string_duplicate(config_get_string_value(config, "COORDINATOR_IP"));

	set_coordinator_connection_params(coordinator_ip, coordinator_port);

	config_destroy(config);
	dictionary_destroy(algorithms);

	log_info(logger, "Planifier configuration file loaded");
}

void connect_to_coordinator() {
	coordinator_socket = connect_to(coordinator_ip, coordinator_port);

	if (coordinator_socket < 0) {
		exit_with_error(coordinator_socket, "No se pudo conectar al coordinador");
	} else if (send_module_connected(coordinator_socket, PLANIFIER) < 0) {
		exit_with_error(coordinator_socket, "No se pudo enviar al confirmacion al coordinador");
	} else {
		message_type message_type;
		int message_type_result = recv(coordinator_socket, &message_type,
				sizeof(message_type), MSG_WAITALL);

		if (message_type_result < 0 || message_type != CONNECTION_SUCCESS) {
			exit_with_error(coordinator_socket, "Error al recibir confirmacion del coordinador");
		} else {
			log_info(logger, "Connexion con el coordinador establecida");
		}

		int operation;
		while(recv_sentence_operation(coordinator_socket, &operation) > 0){//todo que condicion pongo aca?
			char* resource;
			get_resource(&resource);
			long esi_id;
			get_esi_id(&esi_id);

			execution_result result;
			switch(operation){
				case GET_SENTENCE:
					try_to_block_resource(resource, esi_id);
					break;
				case SET_SENTENCE:
					if (strcmp(get_resource_taken_by_esi(esi_id), resource) == 0) {
						result = OK;
					}else{
						result = KEY_LOCK_NOT_ACQUIRED;
					}
					send(coordinator_socket, &result, sizeof(execution_result), 0);
					break;
				case STORE_SENTENCE:
					free_resource(resource);
					send_execution_result_to_coordinator(OK);
					break;
				case KEY_UNREACHABLE:
					result = OK;
					send(coordinator_socket, &result, sizeof(execution_result), 0);
					break;
				default:
					log_info(logger, "Connection was received but the operation its not supported. Ignoring");
					break;
			}
		}
	}
}

int get_resource(char** resource){
	if(recv_string(coordinator_socket, resource) <= 0){
		log_info(logger, "Could not get the resource");
		//TODO que hago si no lo pude recibir?
		return 0;
	}
	return 1;
}

int get_esi_id(long* esi_id){
	if(recv_long(coordinator_socket, esi_id) < 0){
		log_info(logger, "Could not get the esi id");
		//TODO que hago si no lo pude recibir?
		return 0;
	}
	return 1;
}

void send_execution_result_to_coordinator(execution_result result){
	if(send(coordinator_socket, &result, sizeof(execution_result), 0) <0){
		log_info(logger, "Could not send response to coordinator");
		//TODO que hago si no lo pude recibir?
	}
}

void try_to_block_resource(char* resource, long esi_id){
	log_debug(logger, "Trying to block resource %s for ESI%ld", resource, esi_id);
	if (bloquear_recurso(resource, esi_id)){
		send_execution_result_to_coordinator(OK);
	}else{
		send_execution_result_to_coordinator(KEY_BLOCKED);
	}
}

int main(int argc, char* argv[]) {
	init_logger();
	load_configuration(argv[1]);

	set_orchestrator();
	init_dispatcher();

	int server_started = start_server(server_port, server_max_connections, (void *) esi_connection_handler, true, logger);
	if (server_started < 0) {
		log_error(logger, "Server not started");
		exit_gracefully(EXIT_FAILURE);
	}
	pthread_t console_thread = start_console();
	connect_to_coordinator();
	pthread_join(console_thread, NULL);
	exit_gracefully(EXIT_SUCCESS);
}
