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

void load_configuration(char* config_file_path){
	char* port_name = "COORDINATOR_PORT";
	char* ip = "COORDINATOR_IP";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	coordinator_ip = config_get_string_value(config, ip);
	coordinator_port = config_get_int_value(config, port_name);
	log_info(logger, "OK.");
}

int create_socket(){

}

void exit_gracefully(int return_nr) {
	log_destroy(logger);
	exit(return_nr);
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
	t_instance_configuration *instance_configuration = malloc(sizeof(t_instance_configuration));

	if(recv(socket, instance_configuration, sizeof(t_instance_configuration), 0) <= 0){
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

void send_connection_request(int socket){
	if(send(socket, 0, sizeof(int), 0) <= 0){
		char* error_msg = "Failed sending connection request.";
		log_error(logger, error_msg);
		_exit_with_error(socket, error_msg, NULL);
	}

	log_info(logger, "Connection request received OK !");
}

int main(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing instance...");
	load_configuration(argv[1]);
	int socket_fd = create_socket(); //TODO esto debería estar en nuestro commons
	send_connection_request(socket_fd);
	receive_instance_configuration(socket_fd);
}
