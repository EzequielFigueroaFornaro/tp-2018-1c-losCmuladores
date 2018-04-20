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
		log_error(logger, "Error binding. Server not started.");
		return 1;
	}

	log_info(logger, "Server Started. Listening on port %d", port);
	listen(server_socket, 5); //TODO llevar a cfg

	return server_socket;
}

//TODO llevar a commons
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

	int server_socket = start_server(server_port);
	pthread_t listener_thread;
	if(pthread_create(&listener_thread, NULL, listen_for_instances, (void*) server_socket)){
		log_error(logger, "Error in thread");
		exit(1);
	}

	//close(server_socket);
	pthread_join(listener_thread, NULL);
	return EXIT_SUCCESS;
}
