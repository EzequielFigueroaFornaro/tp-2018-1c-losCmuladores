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
	logger = log_create("coordinator.log", "coordinator", 1, LOG_LEVEL_INFO);
}

void exit_gracefully(int code) {
	log_destroy(logger);
	free(instance_configuration -> entries_quantity);
	free(instance_configuration -> entries_size);
	free(instance_configuration -> operation_id);
	free(instance_configuration);

	list_destroy(instances_thread_list);
	exit(code);
}

void check_server_startup(int server_socket, int port) {
	if (server_socket == 1) {
		_exit_with_error(server_socket, "Error binding. Server not started.", NULL);
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
	server_max_connections = config_get_int_value(config, "MAX_ACCEPTED_CONNECTIONS");

	planifier_ip = config_get_string_value(config, "PLANIFIER_IP");
	planifier_port = config_get_string_value(config, "PLANIFIER_PORT");

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
            if( pthread_create( &instance_thread_id , NULL , (void*)instance_connection_handler, (void*) client_sock) < 0) {
            	log_error(logger, "Could not create thread.");
            }
            log_info(logger, "Connection accepted !");
            //TODO ver qué info necesito, guardar en el struct de la instancia, y hacer free de todo lo necesario.
    }

    //TODO ver si corresponde hacer checkeos.

}

void sig_handler(int signo)
{
	if (signo == SIGSTOP){
		printf("received SIGSTOP\n");
		exit(-1);
		}
	if (signo == SIGKILL){
			printf("received sigkill\n");
			exit(0);
	}
}

void _exit_with_error(int socket,char* error_msg, void * buffer){
	if (buffer != NULL) {
		free(buffer);
	}
	log_error(logger, error_msg);
	close(socket);
	close(planifier_socket);
	exit_gracefully(1);
}

void signal_handler(int sig){
    if (sig == SIGINT) {
    	log_info(logger,"Caught signal for Ctrl+C\n");
    	exit_gracefully(0);
    }
}


int main(int argc, char* argv[]) {
	instances_thread_list = list_create();
	configure_logger();
    signal(SIGINT,signal_handler);
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);
	int server_socket = start_server(server_port, server_max_connections);
	planifier_socket = connect_to_planifier();
	check_server_startup(server_socket, server_port);
	pthread_t listener_thread;
	if(pthread_create(&listener_thread, NULL, (void*)listen_for_instances, (void*) server_socket) < 0){
		_exit_with_error(server_socket, "Error in thread", NULL);
	};

	//close(server_socket);
	pthread_join(listener_thread, NULL);
	return EXIT_SUCCESS;
}

void send_planifier_connection(int planifier_socket) {
	int connection_type = 101;
	int result = send(planifier_socket, &connection_type, sizeof(int), 0);
	if (result < 0) {
		_exit_with_error(planifier_socket, "Error trying to send planifier connection confirmation. Code: " + result, NULL);
	} else {
		log_info(logger, "Confirmacion de conexion con el planificador enviada");
	}
}

void recv_planifier_connection(int planifier_socket) {
	int status;
	int result = recv(planifier_socket, &status, sizeof(int), MSG_WAITALL);
	if (result < 0 || status != 1) {
		_exit_with_error(planifier_socket, "Error trying to receive planifier connection confirmation.", NULL);
	} else {
		log_info(logger, "Confirmacion de conexion con el planificador recibida");
	}
}

int connect_to_planifier() {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(planifier_ip, planifier_port, &hints, &server_info);

	int planifier_socket = socket(server_info -> ai_family, server_info -> ai_socktype, server_info -> ai_protocol);

	int connect_status = connect(planifier_socket, server_info -> ai_addr, server_info -> ai_addrlen);

	freeaddrinfo(server_info);

	if(connect_status == -1){
		log_error(logger, "No se pudo conectar al planificador!");
		exit_gracefully(1);
	} else {
		send_planifier_connection(planifier_socket);
		recv_planifier_connection(planifier_socket);
	}

	log_info(logger, "Conectado al planificador!");
	return planifier_socket;
}
