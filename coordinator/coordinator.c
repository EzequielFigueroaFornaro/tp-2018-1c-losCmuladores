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

//TODO test.
int send_instance_configuration(int client_sock){
	int status = send(client_sock, instance_configuration, sizeof(t_instance_configuration), 0);
	if(status <= 0){
		log_error(logger, "Could not send instance configuration.%d", status);
		return 1;
	}
	return 0;
}


void *instance_connection_handler(int instance_socket){

	//receive_instance_header(instance_socket);
	send_instance_configuration(instance_socket);

}

void listen_for_instances(int server_socket) {
	log_info(logger, "Waiting for instances...");
    struct sockaddr_in client;
	pthread_t instance_thread_id; //TODO ojo con la memoria. Esto es de prueba,
	t_instance *aux_instance = (t_instance*) malloc(sizeof(t_instance)); //TODO hacer free.
	t_list * connected_instances_thread_list;
	connected_instances_thread_list = list_create();

	int client_sock;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	//TODO cómo me guardo el resultado de instance_connection_handler ??
    while( (client_sock = accept(server_socket, (struct sockaddr *)&addr, &addrlen)) ) {
            log_info(logger, "Connection request received.");
            if( pthread_create( &instance_thread_id , NULL ,  instance_connection_handler, (void*) client_sock) < 0) {
            	log_error(logger, "Could not create thread.");
            }

            pthread_join(instance_thread_id, NULL);
            //TODO ver qué info necesito, guardar en el struct de la instancia, y hacer free de todo lo necesario.
            //list_add(connected_instances_thread_list, instance_thread_id);
            log_info(logger, "Connection accepted !");
    }

	log_info(logger, "No espe");


}

int main(int argc, char* argv[]) {
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

