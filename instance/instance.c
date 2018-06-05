/*
 ============================================================================
 Name        : instance.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto instance.
 ============================================================================
 */

#include "instance.h"
#include "instance-test.h"

#include <commons/log.h>
#include <stdbool.h>

bool instance_running = true;

void configure_logger() {
	logger = log_create("instance.log", "instance", true, LOG_LEVEL_INFO);
}

void exit_gracefully(int return_nr) {
	log_destroy(logger);
	entry_table_destroy(entries_table);
	close(coordinator_socket);
	exit(return_nr);
}


t_instance_config* load_configuration(char* config_file_path){
	log_info(logger, "Loading instance configuration file...");

	t_config* config = config_create(config_file_path);

	t_instance_config *instance_config = malloc(sizeof(t_instance_config));
	instance_config->coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	instance_config->coordinator_ip = string_duplicate(config_get_string_value(config, "COORDINATOR_IP"));
	instance_config->instance_name = string_duplicate(config_get_string_value(config, "NAME"));
	instance_config->mount_path = string_duplicate(config_get_string_value(config, "MOUNT_PATH"));
	instance_config->replacement_algorithm = string_duplicate(config_get_string_value(config, "REPLACEMENT_ALGORITHM"));

	config_destroy(config);

	log_info(logger, "Instance configuration loaded OK");
	return instance_config;
}

void _exit_with_error(int socket,char* error_msg, void * buffer){
	if (buffer != NULL) {
		free(buffer);
	}
	log_error(logger, error_msg);
	close(socket);
	exit_gracefully(1);
}

void exit_with_error(int socket, char *error_msg) {
	_exit_with_error(socket, error_msg, NULL);
}

t_instance_configuration *receive_instance_configuration(int socket){
	t_instance_configuration *instance_configuration = (t_instance_configuration*) malloc(sizeof(t_instance_configuration));
	log_info(logger, "Receiving instance configuration from coordinator.");

	int status = recv(socket, instance_configuration, sizeof(t_instance_configuration), MSG_WAITALL);
	if(status <= 0){
		char* error_msg = "Could not receive instance configuration";
		log_error(logger, error_msg);
		_exit_with_error(socket, error_msg, instance_configuration);
	}

	log_info(logger, "Configuration successfully received !");
	return instance_configuration;
}

void check_if_connection_was_ok(int server_socket){
	 if(server_socket == -1){
		  log_error(logger, "Could not connect with coordinator.");
		  exit_gracefully(1);
	  }

	  log_info(logger, "Connected !");
}

int process_sentence(t_sentence* sentence){
	log_info(logger, "Processing statement...");
	switch(sentence->operation_id) {
	case GET_SENTENCE:;
		char *value = entry_table_get(entries_table, sentence->key);
		if (NULL == value) {
			log_info(logger, "Value %s get for key %s", value, sentence->key);
		} else {
			log_info(logger, "Value %s get for key %s", value, sentence->key);
		}
		return 0;
	case SET_SENTENCE:
		return entry_table_put(entries_table, sentence->key, sentence->value);
	case STORE_SENTENCE:
		return entry_table_store(entries_table, instance_config->mount_path, sentence->key);
	default:
		return -1;
	}
}


//TODO hacer deserializador.
t_sentence* wait_for_statement(int socket_fd) {
	log_info(logger, "Waiting for sentence from coordinator...");

	t_sentence *sentence = sentence_create();

	if (recv_sentence_operation(socket_fd, &sentence->operation_id) > 0) {
		if (recv_string(socket_fd, &sentence->key) > 0) {
			if (recv_string(socket_fd, &sentence->value) > 0) {
				log_info(logger, "Sentence successfully received: %s", sentence_to_string(sentence));
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

void signal_handler(int sig){
    if (sig == SIGINT) {
    	log_info(logger, "Caught signal for Ctrl+C\n");
    	instance_running = false;
    }
}

void send_result(int result){

	int send_result = send(coordinator_socket, &result, sizeof(int), 0);

	if(send_result <= 0){
		log_error(logger, "Could not send result to coordinator.");
	}
}

void send_instance_name(int coordinator_socket, char *instance_name) {
	log_info(logger, "Sending Instance name to coordinator...");
	int instance_name_length = strlen(instance_name) + 1;

	int message_size = sizeof(int) + instance_name_length;

	void* buffer = malloc(message_size);
	void* offset = buffer;

	concat_string(&offset, instance_name, instance_name_length);

	send(coordinator_socket, buffer, message_size, 0);

	int confirmation_response;

	int name_response = recv(coordinator_socket, &confirmation_response, sizeof(int), 0);
	if(name_response <= 0){
		_exit_with_error(coordinator_socket, "Could not receive instance name confirmation from coordinator.", buffer);
	}

	if(confirmation_response != 1) {
		_exit_with_error(coordinator_socket, "Instance name error. Maybe other instance with same name is already running.", buffer);
	}
	log_info(logger, "Instance name sent OK.");
}

int instance_run(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing instance...");
    signal(SIGINT,signal_handler);

	instance_config = load_configuration(argv[1]);

	coordinator_socket = connect_to_coordinator(instance_config->coordinator_ip, instance_config->coordinator_port);

	send_instance_name(coordinator_socket, instance_config->instance_name);

	t_instance_configuration *configuration = receive_instance_configuration(coordinator_socket);
	entries_table = entry_table_create(configuration->entries_quantity, configuration->entries_size);

	log_info(logger, "Initializing instance... OK");
	while(instance_running){
		t_sentence* sentence = wait_for_statement(coordinator_socket);
		process_sentence(sentence);
		send_result(200);
		//TODO avisar al coordinador.
	}
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc > 2 && string_equals_ignore_case(argv[2], "--test")) {
		return instance_run_test();
	} else {
		return instance_run(argc, argv);
	}
}

int connect_to_coordinator(char *coordinator_ip, int coordinator_port) {
	log_info(logger, "Connecting with coordinator.");

	int coordinator_socket = connect_to(coordinator_ip, coordinator_port);

	if (coordinator_socket < 0) {
		exit_with_error(coordinator_socket, "No se pudo conectar al coordinador");
	} else if (send_module_connected(coordinator_socket, INSTANCE) < 0) {
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
	}
	return coordinator_socket;
}
