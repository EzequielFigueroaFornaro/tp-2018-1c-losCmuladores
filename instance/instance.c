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
	exit(return_nr);
}


void load_configuration(char* config_file_path){
	char* port_name = "COORDINATOR_PORT";
	char* ip = "COORDINATOR_IP";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	coordinator_ip = config_get_string_value(config, ip);
	coordinator_port = config_get_int_value(config, port_name);
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

void free_header(t_content_header* header){
	free(header -> operation_id);
	free(header -> length);
	free(header);
	return;
}

/*
void wait_for_statement_and_return_result(int socket_fd){
	t_content_header *sentence_header = (t_content_header*) malloc(sizeof(t_content_header));

	int sentence_request_header = recv(socket_fd, sentence_header, sizeof(t_content_header), MSG_WAITALL);
	if (sentence_request_header <= 0){
		log_error(logger, "Could not receive instance configuration");
		free_header(sentence_header);
		return;
	}

	if(sentence_header -> operation_id != 2) {
		log_error(logger, "Invalid operation id...expected %d and was %d", 2, sentence_header -> operation_id);
		free_header(sentence_header);
		return;
	}

	//TODO hacer receive del payload
}
*/

int main(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing instance...");
	load_configuration(argv[1]);
	log_info(logger, "Connecting with coordinator.");
	int socket_fd = connect_to(coordinator_ip, coordinator_port);
	receive_instance_configuration(socket_fd);
	/*while(1){
		wait_for_statement_and_return_result(socket_fd);
	}*/
	exit(0);
}
