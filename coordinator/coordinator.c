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

	log_info(logger, "Server Started. Listening on port %d", port);
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
		return -1; //TODO Ver qué hacer, cómo manejar este error...no debería matar el proceso...
	}

	log_info(logger, "Connection accepted !");
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

//TODO testear.
void receive_instance_header(int socket_fd){
	log_info(logger, "Checking for instance request operation id");
	int operation_id = -1;

	if(recv(socket_fd, operation_id, sizeof(int), 0) <= 0){
		log_error(logger, "Could not receive instance header");
		//TODO cerrar socket, free del header. Return
	}

	//0 es el id definido para request de conexion de una nueva instancia.
	if(operation_id != 0){
		log_error(logger, "Invalid operation Id. Should be 0 and was %d", operation_id);
	}

	log_info(logger, "Valid request operation id.");
}


void load_configuration(char* config_file_path){
	char* port_name = "SERVER_PORT";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	server_port = config_get_int_value(config, port_name);
	//TODO asignar la cfg de instancias a "instance_configuration" declarado en .h .
	log_info(logger, "OK.");
}

//TODO test.
int send_instance_configuration(int client_sock){
	if(send(client_sock, instance_configuration, sizeof(t_instance_configuration), 0) <= 0){
		log_error(logger, "Could not send instance configuration.");
		return 1;
	}
	return 0;
}

void listen_for_instances(int server_socket) {
	log_info(logger, "Waiting for instances...");
	pthread_t instance_thread; //TODO ojo con la memoria. Esto es de prueba,
	t_instance *aux_instance = (t_instance*) malloc(sizeof(t_instance)); //TODO hacer free.
	t_list * connected_instances_thread_list;
	connected_instances_thread_list = list_create();

	while(1){
		int client_sock = accept_connection(server_socket);

		/*if(pthread_create(&instance_thread, NULL, NULL, NULL)){
			log_error(logger, "Error while accepting instance connection.");
		}*/

		receive_instance_header(client_sock);
		send_instance_configuration(client_sock);

		//aux_instance -> socket_id = client_sock;
		//aux_instance -> instance_thread = instance_thread;
		//log_info(logger, "Alloque bien");

//		list_add(connected_instances_thread_list, aux_instance);
	//	log_info(logger, "Guarde bien ne la lista.");

		//free(aux_instance -> instance_thread);
		//free(aux_instance-> socket_id);
		//free(aux_instance);
		log_info(logger, "New Instance !");
		break;
	}
}

/*void* listen_for_instances(int server_socket) {
	log_info(logger, "Waiting for instances...");
	pthread_t aux_instance_thread;

	int client_id;
	while(true){
		if(pthread_create(&aux_instance_thread, NULL, accept_connection(server_socket), &client_id)){
			log_error(logger, "Error al conectar instancia.");//TODO
		}

		list_add(connected_instances_thread_list, aux_instance_thread);
		log_info(logger, "Conexión nueva en thread nuevo !");
	}
}*/

int main(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	int server_socket = start_server(server_port);
	//Llama  escuchar instancias en otro thread para no bloquear el proceso.
	pthread_t listener_thread;
	if(pthread_create(&listener_thread, NULL, listen_for_instances, (void*) server_socket)){
		log_error(logger, "Error in thread");
		exit(1);
	}

	//int client_socket = accept_connection(server_socket);
	//log_info(logger, "Acepte conexión. Client: %d", client_socket);

	//close(server_socket);
	pthread_join(listener_thread, NULL);
	return EXIT_SUCCESS;
}
