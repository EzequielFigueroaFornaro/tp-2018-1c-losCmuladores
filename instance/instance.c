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
	coordinator_port = config_get_string_value(config, port_name);
	log_info(logger, "OK.");
}

int connect_to_server(char * ip, char * port) {
  struct addrinfo hints;
  struct addrinfo *server_info;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  getaddrinfo(ip, port, &hints, &server_info);

  int server_socket = socket(server_info -> ai_family, server_info -> ai_socktype, server_info -> ai_protocol);

  int retorno = connect(server_socket, server_info -> ai_addr, server_info -> ai_addrlen);

  freeaddrinfo(server_info);

  if(retorno == -1){
	  log_error(logger, "No se pudo conectar !");
	  exit_gracefully(1);
  }

  log_info(logger, "Conectado!");
  return server_socket;
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

int main(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing instance...");
	load_configuration(argv[1]);
	//TODO esto debería estar en nuestro commons
	int socket_fd = connect_to_server(coordinator_ip, coordinator_port);
	receive_instance_configuration(socket_fd);
	exit(0);
}
