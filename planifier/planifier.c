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
	dictionary_put(algorithms, "SJF", string_from_format("%d", SJF));
	dictionary_put(algorithms, "SJF_DESALOJO", string_from_format("%d", SJF_DESALOJO));
	dictionary_put(algorithms, "HRRN", string_from_format("%d", HRRN));

	char* algorithm_code = config_get_string_value(config, "ALGORITHM");
	algorithm = atoi(dictionary_get(algorithms, algorithm_code));

	log_debug(logger, "Algorithm is: %d (%s)", algorithm, algorithm_code);

	alpha = config_get_int_value(config, "ALPHA");
	initial_estimation = config_get_int_value(config, "INITIAL_ESTIMATION");

	server_port = config_get_int_value(config, "SERVER_PORT");
	server_max_connections = config_get_int_value(config,
			"MAX_ACCEPTED_CONNECTIONS");

	coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	coordinator_ip = string_duplicate(config_get_string_value(config, "COORDINATOR_IP"));
	set_coordinator_connection_params(coordinator_ip, coordinator_port);

	char* taken_resources = config_get_string_value(config, "TAKEN_RESOURCES");

	log_info(logger, "Planifier configuration file loaded");

	set_orchestrator();
	if (!string_is_blank(taken_resources)) {
		char** splitted_command = string_split(taken_resources, ",");
		char** ptr = splitted_command;
		for (char* c = *ptr; c; c = *++ptr) {
			long esi_id = ESI_BLOQUEADO;
			dictionary_put_id(recurso_tomado_por_esi, c, esi_id);
		}
		//free(splitted_command);
	}
	log_info(logger, "Orchestrator loaded");
	config_destroy(config);
	dictionary_destroy(algorithms);
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

        while(true){//TODO QUE CONDICION PONGO ACA?
			t_planifier_sentence* sentence = wait_for_statement_from_coordinator(coordinator_socket);
            log_info(logger, "Se recibio un nuevo mensaje del coordinador");
            switch(sentence->operation_id){
            	case GET_SENTENCE:
            		log_info(logger, "Tipo de mensaje: GET ; Esi Id: %ld",sentence->esi_id);
                    try_to_block_resource(sentence->resource, sentence->esi_id);
                	break;
                case SET_SENTENCE:
                	log_info(logger, "Tipo de mensaje: SET ; Esi Id: %ld ; Resource: %s", sentence->esi_id, sentence->resource);
                	execution_result result;
                	if (is_resource_taken_by_esi(sentence->esi_id, sentence->resource)) {
                		log_info(logger, "Operacion SET exitosa");
                		result = OK;
                	}else{
                		log_error(logger, "No se pudo realizar el SET");
                		result = KEY_LOCK_NOT_ACQUIRED;
                	}
					send_execution_result_to_coordinator(result);
                	break;
                case STORE_SENTENCE:
                    log_info(logger, "Tipo de mensaje: STORE ; Resource: %s",sentence->resource);
                    free_resource(sentence->resource);
					send_execution_result_to_coordinator(OK);
                    break;
                case KEY_UNREACHABLE:
                    log_info(logger, "Tipo de mensaje: KEY_UNREACHABLE");
					send_execution_result_to_coordinator(OK);
                    break;
                default:
                    log_info(logger, "Connection was received but the operation its not supported. Ignoring");
                    break;
                }
            planifier_sentence_destroy(sentence);
        }
	}
}

t_planifier_sentence* wait_for_statement_from_coordinator(int socket_id) {
    log_info(logger, "Waiting for sentence from coordinator...");

	t_planifier_sentence *sentence = planifier_sentence_create();

    if (recv_int(socket_id, &sentence->operation_id) > 0) {
        if (recv_string(socket_id, &sentence->resource) > 0) {
            if (recv_long(socket_id, &sentence->esi_id) > 0) {
                char *sentence_str = planifier_sentence_to_string(sentence);
                log_info_highlight(logger, "Sentence successfully received: %s", sentence_str);
                free(sentence_str);
                return sentence;
            } else {
                log_error(logger, "Could not receive sentence value.");
            }
        } else {
            log_error(logger, "Could not receive sentence key.");
        }
    } else {
        log_error(logger, "Could not receive sentence operation id.");
    }

    planifier_sentence_destroy(sentence);
    exit_with_error(coordinator_socket, "Error receiving sentence from coordinator. Maybe was disconnected");
}

void send_execution_result_to_coordinator(execution_result result){
	if(send(coordinator_socket, &result, sizeof(int), 0) <0){
		log_error(logger, "Could not send response %d to coordinator",result);
		exit_with_error(coordinator_socket, "Error while sending execution result");
	}
}

void try_to_block_resource(char* resource, long esi_id){
	log_debug(logger, "Trying to block resource %s for ESI%ld", resource, esi_id);
    bool took_resource = bloquear_recurso(resource, esi_id);
	if (took_resource){
		log_debug(logger, "Resource %s assigned to ESI%ld. Sending OK to coordinator", resource, esi_id);
		send_execution_result_to_coordinator(OK);
	}else{
		log_debug(logger, "ESI%ld was added to waiting queue of resource %s. Sending KEY_BLOCKED to coordinator", esi_id, resource);
		send_execution_result_to_coordinator(KEY_BLOCKED);
	}
}

void assert_not_blank(char* msg, char* arg) {
	if (arg == NULL || string_is_empty(arg)) {
		log_error(logger, msg);
		log_destroy(logger);
		exit(1);
	}
}

int main(int argc, char* argv[]) {
	init_logger();

	assert_not_blank("Archivo de configuracion requerido!", argv[1]);
	load_configuration(argv[1]);

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
