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

void iterate_list_for_eq(t_instance* t_instance) {

}

//TODO Seguramente los mutex no vayan acá, sino en donde se orqueste la selección de la instancia,
t_instance* select_instance_to_send_by_equitative_load(){
	bool _is_available(t_instance* instance){
		return instance -> is_available;
	}

	t_instance* selected;

	pthread_mutex_lock(&instances_mtx);

	t_list* available_instances_list = list_filter(instances_thread_list, _is_available);
	t_link_element *element = available_instances_list -> head;
	t_link_element *aux = NULL;

	if(last_instance_selected == NULL){
		selected = (t_instance*)instances_thread_list -> head -> data;
	} else {
		while (element != NULL) {
			aux = element -> next;
			if(element -> data == last_instance_selected){
				selected = aux != NULL ? aux -> data : instances_thread_list -> head -> data;
				break;
			}
			element = aux;
		}
	}
	//TODO ver que liberar, y donde dejar las cosas...
	//free(last_instance_selected);
	//memcpy(last_instance_selected, selected);
	pthread_mutex_unlock(&instances_mtx);

	list_destroy(available_instances_list);

	return selected;
}

//Calcula a cuál mandar la instrucción.
//2)
//Antes de hacer esto hay que verificar que se pueda realizar la operación, sino devolver error al planificador.
t_instance* select_instance_to_send_by_distribution_strategy(char first_char_of_key){
	switch(distribution) {
		case EL: return select_instance_to_send_by_equitative_load();
		case LSU: return NULL;//TODO
		case KE: return NULL; //TODO
		default: return NULL; //TODO
	}
}

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

	pthread_mutex_lock(&instances_mtx);
	list_destroy(instances_thread_list);
	pthread_mutex_unlock(&instances_mtx);
	pthread_mutex_destroy(&instances_mtx);
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
	t_config* config = config_create(config_file_path); //TODO valgrind

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
	} else _exit_with_error(0, "Distribution algorithm not found.", NULL);

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
		instance -> is_available = true;

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


/*void send_instruction_for_test(char* forced_key, char* forced_value){
	//*************************
	//****ESTO ES DE PRUEBA;
	int operation_id = 601;
	char* key = forced_key;
	char* value = forced_value;
	int size = sizeof(operation_id) + strlen(key) + 1 + strlen(value) + 1;
	t_sentence *sentence = malloc(size);
	sentence -> operation_id = operation_id;
	sentence -> key = key;
	sentence -> value = value;

	t_instance* selected_instance = select_instance_to_send_by_distribution_strategy(forced_key[0]);
	//TODO guardarlo en la tabla
	last_instance_selected = selected_instance;
	int last_socket_id = last_instance_selected -> socket_id;
	send_statement_to_instance_and_wait_for_result(last_socket_id, sentence);

			//***********************

}*/

int main(int argc, char* argv[]) {
	instances_thread_list = list_create();
	ise_thread_list = list_create();
	configure_logger();
    signal(SIGINT,signal_handler);
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	int server_socket = start_server(server_port, server_max_connections, (void *)connection_handler, false, logger);
	check_server_startup(server_socket); //TODO llevar esto adentro del start_server ?

	//**TODO TEST***/
	while(instances_thread_list -> elements_count < 3);
	sleep(5);

	/*send_instruction_for_test("barcelona:jugadores", "messi");
	send_instruction_for_test("barcelona:jugadores", "neymar");
	send_instruction_for_test("barcelona:jugadores", "busquets");
	send_instruction_for_test("barcelona:jugadores", "pique");
	send_instruction_for_test("barcelona:jugadores", "iniesta");

*/
	exit_gracefully(EXIT_SUCCESS);
}
