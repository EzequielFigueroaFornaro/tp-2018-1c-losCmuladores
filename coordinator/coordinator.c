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

//TODO recibir modelo de Statement. Recibir acá el resultado, o es async ?
//Recibe solicitud del ESI.
//1)
void receive_statement_request();


//Calcula a cuál mandar la instrucción.
//2)
//Antes de hacer esto hay que verificar que se pueda realizar la operación, sino devolver error al planificador.
int calculate_instance_number_to_send();

//TODO recibir modelo de Statement. Recibir acá el resultado, o es async ?
//3)
int send_statement_to_instance();

//4)
//TODO definir el struct del resultado.
void receive_statement_result_from_instance();

//Devuelve el resultado al ESI.
//5)
void send_statement_result_to_ise();

void configure_logger() {
	logger = log_create("coordinator.log", "coordinator", true, LOG_LEVEL_INFO);
}

void exit_gracefully(int code) {
	log_destroy(logger);
	exit(code);
}

void check_server_startup(int startup_result, int port) {
	if (startup_result == 1) {
		log_error(logger, "Error binding. Server not started.");
		exit_gracefully(1);
	}
	log_info(logger, "Server Started. Listening on port %d", port);
}

void check_accept(int accept_result) {
	if (accept_result == -1) {
		log_error(logger, "Could not accept connection.");
		exit_gracefully(1); // TODO Se supone que no debería matar el proceso
	}
	log_info(logger, "Connection accepted !");
}

void load_configuration(char* config_file_path){
	char* port_name = "SERVER_PORT";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	server_port = config_get_int_value(config, port_name);
	instance_configuration = malloc(sizeof(t_instance_configuration));
	instance_configuration -> operation_id = 1;
	instance_configuration -> entries_quantity = 100;
	instance_configuration -> entries_size = 12;
	//TODO asignar la cfg de instancias a "instance_configuration" declarado en .h .
	log_info(logger, "OK.");
}

int send_instance_configuration(int client_sock){
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	getpeername(client_sock, (struct sockaddr *)&addr, &addr_size);

	log_info(logger, "Sending instance configuration to host %s:%d", inet_ntoa(addr.sin_addr), (int) ntohs(addr.sin_port));
	int status = send(client_sock, instance_configuration, sizeof(t_instance_configuration), 0);
	if(status <= 0){
		log_error(logger, "Could not send instance configuration.%d", status);
		close(client_sock);
		return 1;
	}
	log_info(logger, "Configuration successfully sent.");
	return 0;
}


void *instance_connection_handler(int instance_socket){

	//receive_instance_header(instance_socket);
	send_instance_configuration(instance_socket);

	t_instance *instance = (t_instance*) malloc(sizeof(t_instance));

	instance -> instance_thread = pthread_self();
	instance -> socket_id = instance_socket;
	list_add(instances_thread_list, instance);

}

void listen_for_instances(int server_socket) {
	log_info(logger, "Waiting for instances...");
	pthread_t instance_thread_id;

	int client_sock;

	//TODO revisar si realmente necesito esto acá. Lo uso en la función que envía la configuración.
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

    while( (client_sock = accept(server_socket, (struct sockaddr *)&addr, &addrlen)) ) {
            log_info(logger, "Connection request received.");
            if( pthread_create( &instance_thread_id , NULL ,  instance_connection_handler, (void*) client_sock) < 0) {
            	log_error(logger, "Could not create thread.");
            }
            log_info(logger, "Connection accepted !");
            //TODO ver qué info necesito, guardar en el struct de la instancia, y hacer free de todo lo necesario.
    }

    //TODO ver si corresponde hacer checkeos.

}

int main(int argc, char* argv[]) {
	instances_thread_list = list_create();
	configure_logger();
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	int server_socket = start_server(server_port, 5); // TODO Llevar a conf
	check_server_startup(server_socket, server_port);
	pthread_t listener_thread;
	if(pthread_create(&listener_thread, NULL, listen_for_instances, (void*) server_socket)){
		log_error(logger, "Error in thread");
		exit(1);
	}

	//close(server_socket);
	pthread_join(listener_thread, NULL);
	return EXIT_SUCCESS;
}

