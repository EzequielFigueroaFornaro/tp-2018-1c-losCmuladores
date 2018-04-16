/*
 ============================================================================
 Name        : coordinator.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto coordinator
 ============================================================================
 */

#include "coordinator.h"

void configure_logger() {
	logger = log_create("coordinator.log", "coordinator", true, LOG_LEVEL_INFO);
}

//TODO llevar a commons
int start_server(int port){
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	int enabled = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

	if (bind(server_socket, (void*) &server_address, sizeof(server_address)) != 0){
		perror("Error binding. Server not started.");
		return 1;
	}

	log_info(logger, "Listening on port %d", port);
	listen(server_socket, 100);

	return server_socket;
}

//TODO llevar a commons
//TODO levantar threads por c/conexion aceptada.
int accept_connection(int server_socket){
	struct sockaddr_in cli_addr;
	socklen_t address_size;

	address_size = sizeof(cli_addr);
	int client_socket = accept(server_socket, (void*) &cli_addr, &address_size);

	if (client_socket == -1){
		log_error(logger, "Could not accept connection.");
		return -1; //TODO acá habría que cerrar socket con algúna llamada a algo de commons y matar el proceso.
	}

	return client_socket;
}

//TODO recibir modelo de Statement. Recibir acá el resultado, o es async ?
//Recibe solicitud del ESI.
//1)
void receive_statement_request(){

}

//Calcula a cuál mandar la instrucción.
//2)
//Antes de hacer esto hay que verificar que se pueda realizar la operación, sino devolver error al planificador.
int calculate_instance_number_to_send(){
	return 0;//TODO
}

//TODO recibir modelo de Statement. Recibir acá el resultado, o es async ?
//3)
int send_statement_to_instance(){

	//TODO loggear respuesta de la instancia.
}

//4)
//TODO definir el struct del resultado.
void receive_statement_result_from_instance(){

}

//Devuelve el resultado al ESI.
//5)
void send_statement_result_to_ise(){

}

//Manda la configuración correspondiente a la instancia que se acaba de conectar.
void send_configuration_to_instance(int instance_socket){
	//TODO
}


//TODO terminar de levantar la cfg que sea necesaria.
void load_configuration(char* config_file_path){
	char* port_name = "SERVER_PORT";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	server_port = config_get_int_value(config, port_name);
	log_info(logger, "OK.");
}

int main(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);
	int server_socket = start_server(server_port);
	int client_socket = accept_connection(server_socket);


	log_info(logger, "Acepte conexión. Client: %d", client_socket);
	close(server_socket);
	return EXIT_SUCCESS;
}
