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

void configure_logger() {
	logger = log_create("instance.log", "instance", true, LOG_LEVEL_INFO);
}

void exit_gracefully(int return_nr) {
	log_destroy(logger);
	dictionary_destroy(entries_table);
	close(coordinator_socket);
	exit(return_nr);
}


void load_configuration(char* config_file_path){
	char* port_name = "COORDINATOR_PORT";
	char* ip = "COORDINATOR_IP";
	char* name = "NAME";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	coordinator_ip = config_get_string_value(config, ip);
	coordinator_port = config_get_int_value(config, port_name);
	instance_name = config_get_string_value(config, name);

	entries_table = dictionary_create();

	log_info(logger, "OK.");
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

void receive_instance_configuration(int socket){
	t_instance_configuration *instance_configuration = (t_instance_configuration*) malloc(sizeof(t_instance_configuration));
	log_info(logger, "Receiving instance configuration from coordinator.");

	int status = recv(socket, instance_configuration, sizeof(t_instance_configuration), MSG_WAITALL);
	if(status <= 0){
		char* error_msg = "Could not receive instance configuration";
		log_error(logger, error_msg);
		_exit_with_error(socket, error_msg, instance_configuration);
	}

	if (instance_configuration -> operation_id != 1){
		char* error_msg = "Invalid operation Id. Could not receive instance configuration";
		log_error(logger, error_msg);
		_exit_with_error(socket, error_msg, instance_configuration);
	}

	log_info(logger, "Configuration successfully received !");
}

void check_if_connection_was_ok(int server_socket){
	 if(server_socket == -1){
		  log_error(logger, "Could not connect with coordinator.");
		  exit_gracefully(1);
	  }

	  log_info(logger, "Connected !");
}

//TODO
int process_sentence(t_sentence* sentence){
	log_info(logger, "Processing statement...");
	log_info(logger, "FALTA IMPLEMENTAR...");
	return 0;
}


//TODO hacer deserializador.
t_sentence* wait_for_statement(int socket_fd) {
	log_info(logger, "Waiting for Sentence...");

	int operation_id;
	char* key_buffer;
	char* value_buffer;

	if (recv_sentence_operation(socket_fd, &operation_id) > 0) {
		if (recv_string(socket_fd, &key_buffer) > 0) {
			if (recv_string(socket_fd, &value_buffer) > 0) {
				log_info(logger, "Sentence successfully received.");

				t_sentence* sentence = malloc(sizeof(t_sentence));
				sentence -> operation_id = operation_id;
				sentence -> key = key_buffer;
				sentence -> value = value_buffer;
				return sentence;
			} else {
				free(key_buffer);
				log_error(logger, "Could not receive sentence value.");
			}
		} else {
			log_error(logger, "Could not receive sentence key.");
		}
	} else {
		log_error(logger, "Could not receive sentence operation id.");
	}

	return NULL;
}

void signal_handler(int sig){
    if (sig == SIGINT) {
    	log_info(logger,"Caught signal for Ctrl+C\n");
    	exit_gracefully(0);
    }
}

void send_result(int result){

	int send_result = send(coordinator_socket, &result, sizeof(int), 0);

	if(send_result <= 0){
		log_error(logger, "Could not send result to coordinator.");
	}
}

void send_instance_name(){
	log_info(logger, "Sending Instance name to coordinator.");
	int instance_name_length = strlen(instance_name) + 1;

	int message_size = sizeof(int) + instance_name_length;

	void* buffer = malloc(message_size);
	void* offset = buffer;

	concat_string(&offset, instance_name, instance_name_length);

	int result = send(coordinator_socket, buffer, message_size, 0);

	int confirmation_response;

	int name_response = recv(coordinator_socket, &confirmation_response, sizeof(int), 0);
	if(name_response <= 0){
		_exit_with_error(coordinator_socket, "Could not receive instance name confirmation from coordinator.", buffer);
	}

	if(confirmation_response != 1) {
		_exit_with_error(coordinator_socket, "Instance name error. Maybe other instance with same name is already running.", buffer);
	}
	log_info(logger, "Instance name OK.");
}


int main(int argc, char* argv[]) {
	configure_logger();
    signal(SIGINT,signal_handler);
	log_info(logger, "Initializing instance...");
	load_configuration(argv[1]);

	connect_to_coordinator();

	send_instance_name();

	receive_instance_configuration(coordinator_socket);

	while(true){
		t_sentence* sentence = wait_for_statement(coordinator_socket);
		process_sentence(sentence); //TODO obtener resultado.
		send_result(200);
		//TODO avisar al coordinador.
	}

	exit(0);
}

void connect_to_coordinator() {
	log_info(logger, "Connecting with coordinator.");

	coordinator_socket = connect_to(coordinator_ip, coordinator_port);

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
}
