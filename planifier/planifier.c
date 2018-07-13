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

//bool deshabilitar_recurso(char* recurso, long esi_id_desabilitado){
//	// TODO [Lu] revisar &blocked_esi_type
//	pthread_mutex_lock(&blocked_by_resource_map_mtx);
//	long blocked_esi_type = ESI_BLOQUEADO;
//	if(!dictionary_has_key(recurso_tomado_por_esi, recurso)){
//		dictionary_put(recurso_tomado_por_esi, recurso, &blocked_esi_type);
//		dictionary_put(esis_bloqueados_por_recurso, recurso, queue_create());
//		pthread_mutex_unlock(&blocked_by_resource_map_mtx);
//		return true;
//	}
//	long* esi_id = dictionary_get(recurso_tomado_por_esi,recurso);
//	if(*esi_id != esi_id_desabilitado){
//		t_queue* cola_de_esis = dictionary_get(esis_bloqueados_por_recurso,recurso);
//		queue_push(cola_de_esis, esi_id);
////		stop_and_block_esi(esi_id);
//	}
//	finish_esi(*esi_id);
//	dictionary_put(recurso_tomado_por_esi, recurso, &blocked_esi_type);
//	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
//}

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

//		int operation;
/*		while(recv_sentence_operation(coordinator_socket, &operation) > 0){//todo que condicion pongo aca?
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
					break;
				case KEY_UNREACHABLE:
					result = OK;
					send(coordinator_socket, &result, sizeof(execution_result), 0);
					break;
				default:
					log_info(logger, "Connection was received but the operation its not supported. Ignoring");
					break;
			}
		}*/


        while(true){//TODO QUE CONDICION PONGO ACA?
			t_planifier_sentence* sentence = wait_for_statement_from_coordinator(coordinator_socket);
            if (NULL != sentence) {//no hace falta
                log_info(logger, "Se recibio un nuevo mensaje del coordinador");
                switch(sentence->operation_id){
                    case GET_SENTENCE:
                        log_info(logger, "Tipo de mensaje: GET ; Esi Id: %ld",sentence->esi_id);
                        try_to_block_resource(sentence->resource, sentence->esi_id);
                        break;
                    case SET_SENTENCE:
                        log_info(logger, "Tipo de mensaje: SET ; Esi Id: %ld ; Resource: %s", sentence->esi_id, sentence->resource);
                        execution_result result;
                        if (strcmp(get_resource_taken_by_esi(sentence->esi_id), sentence->resource) == 1){
                            log_info(logger, "Operacion SET exitosa");
                            result = OK;
                        }else{
                            log_error(logger, "No se pudo realizar el SET");
                            result = KEY_LOCK_NOT_ACQUIRED;
                        }
                        send(coordinator_socket, &result, sizeof(execution_result), 0);
                        break;
                    case STORE_SENTENCE:
                        log_info(logger, "Tipo de mensaje: STORE ; Resource: %s",sentence->resource);
                        free_resource(sentence->resource);
                        break;
                    case KEY_UNREACHABLE:
                        log_info(logger, "Tipo de mensaje: KEY_UNREACHABLE");
                        send(coordinator_socket, OK, sizeof(execution_result), 0);
                        break;
                    default:
                        log_info(logger, "Connection was received but the operation its not supported. Ignoring");
                        break;
                }
                sentence_destroy(sentence);
            } else {
                exit_with_error(coordinator_socket, "Error receiving sentence from coordinator. Maybe was disconnected");
            }
        }


	}
}

t_planifier_sentence* wait_for_statement_from_coordinator(int socket_id) {
    log_info(logger, "Waiting for sentence from coordinator...");

	t_planifier_sentence *sentence = planifier_sentence_create();

    if (recv_sentence_operation(socket_id, &sentence->operation_id) > 0) {
        if (recv_string(socket_id, &sentence->resource) > 0) {
            if (recv_long(socket_id, &sentence->esi_id) > 0) {
                char *sentence_str = planifier_sentence_to_string(sentence);
                log_info(logger, "Sentence successfully received: %s", sentence_str);
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

    sentence_destroy(sentence);
    return NULL;
}

//int get_resource(char** resource){
//	if(recv_string(coordinator_socket, resource) <= 0){
//		log_info(logger, "Could not get the resource");
//		//TODO que hago si no lo pude recibir?
//		return 0;
//	}
//	return 1;
//}

/*int get_esi_id(long* esi_id){
	if(recv_long(coordinator_socket, esi_id) < 0){
		log_info(logger, "Could not get the esi id");
		//TODO que hago si no lo pude recibir?
		return 0;
	}
	return 1;
}*/

void send_execution_result_to_coordinator(execution_result result){
	if(send(coordinator_socket, &result, sizeof(execution_result), 0) <0){
		log_info(logger, "Could not send response to coordinator");
		//TODO que hago si no lo pude recibir?
	}
}

void free_resource(char* resource){
	pthread_mutex_lock(&blocked_by_resource_map_mtx);
	//TODO ver que onda desbloqueo todo o una sola. UPDATE: habiamos quedado en desbloquear solo una, no?
	dictionary_remove(recurso_tomado_por_esi, resource);
	t_queue* esi_queue = dictionary_get(esis_bloqueados_por_recurso, resource);
	long* esi_id = queue_pop(esi_queue);
	while (!is_valid_esi(*esi_id)) {
		esi_id = queue_pop(esi_queue);
	}
	cambiar_recurso_que_lo_bloquea("", *esi_id);
	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
    //add_esi_bloqueada(*esi_id); TODO ?????
	//TODO OJO AL PIOJO el free de datos como el id que guardamos de la esi bloqueada;
	send_execution_result_to_coordinator(OK);
}

void try_to_block_resource(char* resource, long esi_id){
	log_debug(logger, "Trying to block resource %s for ESI%ld", resource, esi_id);
    acquire_permission_to_block();
    bool took_resource = bloquear_recurso(resource, esi_id);
    release_permission_to_block();
	if (took_resource){
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
