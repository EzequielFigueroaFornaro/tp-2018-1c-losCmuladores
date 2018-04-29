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
//TODO Hacer los free correspondientes!!!
int send_statement_to_instance_and_wait_for_result(int instance_fd, t_sentence *sentence){
	//Antes de hacer esto, guardar en la tabla correspondiente en qué instancia quedó esta key...
	log_info(logger, "Sending sentence to instance...");

	t_buffer* buffer = serialize_sentence(sentence);

	int send_result = send(instance_fd, buffer -> buffer_content, buffer -> size, 0);
	destroy_buffer(buffer);

	if (send_result <= 0) {
		log_error(logger, "Could not send sentence operation id to instance.");
	}

	return 0;
}

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
	free(instance_configuration);

	list_destroy(instances_thread_list);
	exit(code);
}

void check_server_startup(int server_socket) {
	if (server_socket == -1) {
		log_error(logger, "Server not started");
		exit_gracefully(1);
	}
	log_info(logger, "Server Started. Listening on port %d", server_port);
}

void load_configuration(char* config_file_path){
	char* port_name = "SERVER_PORT";

	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	server_port = config_get_int_value(config, port_name);
	server_max_connections = config_get_int_value(config, "MAX_ACCEPTED_CONNECTIONS");

	instance_configuration = malloc(sizeof(t_instance_configuration));
	instance_configuration -> operation_id = 1;
	instance_configuration -> entries_quantity = config_get_int_value(config, "ENTRIES_QUANTITY");
	instance_configuration -> entries_size = config_get_int_value(config, "ENTRIES_SIZE");
	char* distribution_str = config_get_string_value(config, "DISTRIBUTION");
	if(string_equals_ignore_case(distribution_str, "EL")){
		distribution = EL;
	} else if(string_equals_ignore_case(distribution_str, "LSU")) {
		distribution = LSU;
	} else if (string_equals_ignore_case(distribution_str, "KE")){
		distribution = KE;
	} else _exit_with_error(NULL, "Distribution algorithm not found.", NULL);

	log_info(logger, "OK.");
}

int send_instance_configuration(int client_sock){
	log_info(logger, "Sending instance configuration to host %s", get_client_address(client_sock));
	int status = send(client_sock, instance_configuration, sizeof(t_instance_configuration), 0);
	if(status <= 0){
		log_error(logger, "Could not send instance configuration.%d", status);
		close(client_sock);
		return 1;
	}
	log_info(logger, "Configuration successfully sent.");

	//*************************
	//****TODO BORRAR !! ESTO ES DE PRUEBA;
	int operation_id = 601;
	char* key = "barcelona:jugadores";
	char* value = "messi";
	int size = sizeof(operation_id) + strlen(key) + 1 + strlen(value) + 1;
	t_sentence *sentence = malloc(size);
	sentence -> operation_id = operation_id;
	sentence -> key = key;
	sentence -> value = value;
	send_statement_to_instance_and_wait_for_result(client_sock, sentence);
	//***********************

	return 0;
}

void instance_connection_handler(int socket) {
	//TODO ver qué info necesito, guardar en el struct de la instancia, y hacer free de todo lo necesario.
	if (send_connection_success(socket) < 0) {
		_exit_with_error(socket, "Error sending instance connection success", NULL);
	} else {
		send_instance_configuration(socket);

		t_instance *instance = (t_instance*) malloc(sizeof(t_instance));

		instance -> instance_thread = pthread_self();
		instance -> socket_id = socket;
		list_add(instances_thread_list, instance);

		log_info(logger, "Instance connected");
	}
}

void planifier_connection_handler(int socket) {
	planifier_socket = socket;

	if (send_connection_success(socket) < 0) {
		_exit_with_error(socket, "Error sending planifier connection success", NULL);
	} else {
		log_info(logger, "Planifier connected");
	}
}

void ise_connection_handler(int socket) {
	if (send_connection_success(socket) < 0) {
		_exit_with_error(socket, "Error sending ESI connection success", NULL);
	} else {
		t_ise *ise = (t_ise*) malloc(sizeof(t_ise));

		ise -> ise_thread = pthread_self();
		ise -> socket_id = socket;
		list_add(ise_thread_list, ise);

		log_info(logger, "ESI connected");
	}
}

void connection_handler(int socket) {
	message_type message_type;
	int result_connected_message = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);

	if (result_connected_message < 0 || message_type != MODULE_CONNECTED) {
		_exit_with_error(socket, "Error receiving connect message", NULL);
	} else {
		module_type module_type;
		int result_module = recv(socket, &module_type, sizeof(module_type), MSG_WAITALL);
		if (result_module < 0) {
			_exit_with_error(socket, "Error receiving module type connected", NULL);
		} else if (module_type == INSTANCE) {
			instance_connection_handler(socket);
		} else if (module_type == PLANIFIER) {
			planifier_connection_handler(socket);
		} else if (module_type == ISE) {
			ise_connection_handler(socket);
		}
	}
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

void _exit_with_error(int socket, char* error_msg, void * buffer){
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
	ise_thread_list = list_create();
	configure_logger();
    signal(SIGINT,signal_handler);
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	int server_socket = start_server(server_port, server_max_connections, (void *)connection_handler, false, logger);
	check_server_startup(server_socket);

	return EXIT_SUCCESS;
}
