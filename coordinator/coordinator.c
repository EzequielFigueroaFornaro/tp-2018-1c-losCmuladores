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

/* TEST
bool test_block = true;
void test_sentence_result(t_sentence* sentence, int socket, long ise_id);*/

//Recibe solicitud del ESI.
//1)
int receive_sentence_execution_request(int ise_socket, t_sentence** sentence) {
	*sentence = malloc(sizeof(t_sentence));
	int result;
	if ((result = recv_sentence_operation(ise_socket, &((*sentence)->operation_id))) > 0) {
		if ((result = recv_string(ise_socket, &((*sentence)->key))) > 0) {
			result = recv_string(ise_socket, &((*sentence)->value));
		}
	}
	return result;
}

bool is_same_instance(t_instance *one, t_instance *another){
	bool result = one -> instance_thread == another -> instance_thread && one -> socket_id == another -> socket_id;
	return result;
}

//TODO Seguramente los mutex no vayan acá, sino en donde se orqueste la selección de la instancia,
//TODO qué hacemos si no hay ninguna instancia disponible ?
t_instance* select_instance_to_send_by_equitative_load(){
	t_instance* selected = NULL;

	pthread_mutex_lock(&instances_mtx);

	t_link_element *element = instances_thread_list -> head;

	bool is_selected = false;

	if(last_instance_selected != NULL) {
		while (element != NULL && !is_same_instance(element -> data, last_instance_selected)) { //Recorro hasta llegar a la ultima seleccionada.
			element = element -> next;
		}
	} else {
		if (((t_instance*) element -> data) -> is_available){
			selected = (t_instance*) instances_thread_list -> head -> data;
			is_selected = true;
		}
	}

	element = element -> next;
	//Recorro hasta encontrar una disponible...si se termina la lista agarro la primera y vuelvo a checkear.
	while(!is_selected) {
		if (element != NULL){
			if(((t_instance*) element -> data) -> is_available) {
				selected = element -> data;
				is_selected = true;
			}
			element = element -> next;
		} else {
			element = instances_thread_list -> head;
		}
	}

	free(last_instance_selected);
	last_instance_selected = malloc(sizeof(t_instance));
	memcpy(last_instance_selected, selected, sizeof(t_instance));
	pthread_mutex_unlock(&instances_mtx);

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


void handle_instance_disconnection(t_instance* instance){
	pthread_mutex_lock(&instances_mtx);
	instance -> is_available = false;
	pthread_mutex_unlock(&instances_mtx);
	last_instance_selected -> is_available = false; //TODO poner semaforo acá.
	log_info(logger, "Instance %s has been marked as UNAVAILABLE", instance -> ip_port);
}

//3)
//TODO Hacer los free correspondientes!!!
int send_statement_to_instance_and_wait_for_result(t_instance* instance, t_sentence *sentence){
	//Antes de hacer esto, guardar en la tabla correspondiente en qué instancia quedó esta key...
	log_info(logger, "Sending sentence to instance...");

	t_buffer buffer = serialize_sentence(sentence);

	int send_result = send(instance -> socket_id, buffer.buffer_content, buffer.size, 0);
	destroy_buffer(buffer);

	if (send_result <= 0) {
		log_error(logger, "Could not send sentence operation id to instance.");
	}

	int result;
	int result_response = recv(instance -> socket_id, &result, sizeof(int), 0);

	if(result_response == 0) {
		log_error(logger, "Selected instance is not available !");
		handle_instance_disconnection(instance);
		return -1;
	}

	//Put key -> instance.
	pthread_mutex_lock(&keys_mtx);
	dictionary_put(keys_location, sentence -> key, instance);
	pthread_mutex_unlock(&keys_mtx);

	return 0;
}

void save_operation_log(t_sentence* sentence, long ise_id){
	char* string_to_save = string_new();

	char* operation = get_operation_as_string(sentence -> operation_id);

	string_append(&string_to_save, "ESI");
	string_append_with_format(&string_to_save, "%ld | ", ise_id);
	string_append_with_format(&string_to_save, "%s | ", operation);
	string_append_with_format(&string_to_save, "%s", sentence -> key);
	if(sentence -> value != NULL && !string_is_empty(sentence -> value)){
		string_append_with_format(&string_to_save, " %s", sentence -> value);
	}
	string_append(&string_to_save, "\n");

	log_info(logger, "Saving operations log with: %s", string_to_save);

	pthread_mutex_lock(&operations_log_file_mtx);
	operations_log_file = txt_open_for_append(OPERATIONS_LOG_PATH);
	if (operations_log_file == NULL) {
		log_error(logger, "Error saving operation.");
		return;
	}

	txt_write_in_file(operations_log_file, string_to_save);
	txt_close_file(operations_log_file);
	pthread_mutex_unlock(&operations_log_file_mtx);

	free(string_to_save);
	log_info(logger, "Operations log successfully saved");
}

//4)
//TODO definir el struct del resultado.
void receive_statement_result_from_instance();

//Devuelve el resultado al ESI.
//5)
void send_statement_result_to_ise(int socket, long ise_id, execution_result result) {
	int message_size = sizeof(message_type) + sizeof(int);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &EXECUTION_RESULT, sizeof(message_type));
	concat_value(&offset, &result, sizeof(int));
	int send_result = send(socket, buffer, message_size, 0);
	free(buffer);

	if (send_result <= 0) {
		log_error(logger, "Could not send sentence execution result to ESI %s",
				ise_id);
	}
}

void configure_logger() {
	logger = log_create("coordinator.log", "coordinator", 1, LOG_LEVEL_INFO);
}

void exit_gracefully(int code) {
	config_destroy(config);
	log_destroy(logger);
	free(instance_configuration);

	pthread_mutex_lock(&instances_mtx);
	list_destroy(instances_thread_list);
	pthread_mutex_unlock(&instances_mtx);
	pthread_mutex_destroy(&instances_mtx);
	pthread_mutex_destroy(&keys_mtx);
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
	config = config_create(config_file_path);

	server_port = config_get_int_value(config, port_name);
	server_max_connections = config_get_int_value(config, "MAX_ACCEPTED_CONNECTIONS");

	instance_configuration = malloc(sizeof(t_instance_configuration));
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

	keys_location = dictionary_create();

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

void check_if_exists_or_create_new_instance(char* instance_name, int socket){
	bool _is_same_instance_name(t_instance* instance){
		return strcmp(instance -> name, instance_name) == 0;
	}

	t_instance* instance;
	instance = (t_instance*) list_find(instances_thread_list, _is_same_instance_name);

	if(instance != NULL){
		instance -> is_available = true;
	} else {
		instance = (t_instance*) malloc(sizeof(t_instance)); //TODO valgrind

		instance -> instance_thread = pthread_self();
		instance -> socket_id = socket;
		instance -> is_available = true;
		instance -> ip_port = get_client_address(socket);
		instance -> name = instance_name;

		list_add(instances_thread_list, instance);
	}

}

void instance_connection_handler(int socket) {
	char* instance_name;

	bool _is_existent_instance_connected(t_instance* instance){
		return strcmp(instance -> name, instance_name) == 0 && instance -> is_available == true;
	}

	//TODO ver qué info necesito, guardar en el struct de la instancia, y hacer free de todo lo necesario.
	if (send_connection_success(socket) < 0) {
		_exit_with_error(socket, "Error sending instance connection success", NULL);
	} else {

		int instance_name_result = recv_string(socket, &instance_name);

		if(instance_name_result <= 0){
			close(socket);
			log_error(logger, "Could not receive instance name");
			pthread_exit(pthread_self);//todo EXIT THREAD WITH ERROR.
			//_exit_with_error(socket, "Could not receive instance name", NULL);
		}

		if(list_any_satisfy(instances_thread_list, _is_existent_instance_connected)){
			close(socket);
			log_error(logger, "Another instance with same name is connected.");
			pthread_exit(pthread_self);//todo EXIT THREAD WITH ERROR.
						//_exit_with_error(socket, "Could not receive instance name", NULL);
		}

		int r = 1;
		int send_confirmation_result = send(socket, &r, sizeof(int), 0);

		int result = send_instance_configuration(socket);

		if(result != 0){
			log_error(logger, "Error while connecting instance.");
			return;
		}

		check_if_exists_or_create_new_instance(instance_name, socket);

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
	long ise_id;
	log_info(logger, "Socket del ESI: %d", socket);
	if (recv_long(socket, &ise_id) <= 0) {
		log_error(logger, "Could not receive ESI id");
		return;
	}

	if (send_connection_success(socket) < 0) {
		log_error(logger, "Error sending connection success to ESI %ld", ise_id);
		return;
	}

	log_info(logger, "ESI %ld connected", ise_id);

	log_info(logger, "Waiting for first statement...");
	t_sentence* sentence;
	int sentence_received;
	while ((sentence_received = receive_sentence_execution_request(socket, &sentence))) {
		if (sentence_received == MODULE_DISCONNECTED) {
			log_error(logger, "ESI %ld disconnected", ise_id);
			return;
		}
		if (sentence_received < 0) {
			log_error(logger, "Could not receive sentence from ESI %ld", ise_id);
			return;
		}

		log_info(logger, "Sentence received: %s", sentence_to_string(sentence));
		save_operation_log(sentence, ise_id);

		/* TEST: devuelve al primer GET, key_blocked, en los siguientes y para cualquier
		  otra sentencia, devuelve OK al ESI (no hay comunicación con el planificador):

		test_sentence_result(sentence, socket, ise_id);   */

		// TODO: Acciones a ejecutar ante tipo de sentencia

		/* TODO: Descomentar cuando ya se tenga el resultado:
		send_statement_result_to_ise(socket, ise_id, execution_result); */

		free(sentence);
	}
}

void connection_handler(int socket) {
	message_type message_type;
	int result_connected_message = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);
	if (result_connected_message <= 0 || message_type != MODULE_CONNECTED) {
		log_error(logger, "Error receiving connect message");
	} else {
		module_type module_type;
		int result_module = recv(socket, &module_type, sizeof(module_type), MSG_WAITALL);
		if (result_module <= 0) {
			log_error(logger, "Error receiving module type connected");
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


void send_instruction_for_test(char* forced_key, char* forced_value, t_ise* ise){
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

	int resullt = send_statement_to_instance_and_wait_for_result(selected_instance, sentence);

	/*if(send_statement_to_instance_and_wait_for_result(selected_instance, sentence) == -1) {
		//if(sentence -> operation_id == GET_SENTENCE)
		/*Lalala, acá deberíamos ver qué es lo que se está tratando de hacer...
		 - Si es GET, vamos a otra instancia no pasa nada
		 - Si es SET o STORE, tendríamos que avisar al planificador para qeu aborte el ESI correspondiente.

	}*/
	save_operation_log(sentence, ise->id);

			//***********************

}

int main(int argc, char* argv[]) {
	instances_thread_list = list_create();
	ise_thread_list = list_create();
	configure_logger();
    signal(SIGINT,signal_handler);
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	int server_socket = start_server(server_port, server_max_connections, (void *)connection_handler, false, logger);
	check_server_startup(server_socket); //TODO llevar esto adentro del start_server ?

	//**PARA TEST***/
	/*while(instances_thread_list -> elements_count < 3);

	t_ise* ise1 = malloc(sizeof(t_ise));
	ise1 -> id = 1;

	t_ise* ise2 = malloc(sizeof(t_ise));
	ise2 -> id = 2;

	t_ise* ise3 = malloc(sizeof(t_ise));
	ise3 -> id = 3;

	send_instruction_for_test("barcelona:jugadores", "messi", ise1);
	send_instruction_for_test("barcelona:jugadores", "neymar", ise2);
	send_instruction_for_test("barcelona:jugadores", "busquets", ise3);
	send_instruction_for_test("barcelona:jugadores", "pique", ise3);
	send_instruction_for_test("barcelona:jugadores", "iniesta", ise2);

	sleep(6000);
*/
	exit_gracefully(EXIT_SUCCESS);
}

/* TEST
void test_sentence_result(t_sentence* sentence, int socket, long ise_id) {
	if (sentence->operation_id == GET_SENTENCE && test_block) {
		log_info(logger, "[TEST] Sending key_blocked result to esi %ld", ise_id);
		send_statement_result_to_ise2(socket, ise_id, KEY_BLOCKED);
		test_block = false;
	} else {
		log_info(logger, "[TEST] Sending ok result to esi %ld", ise_id);
		send_statement_result_to_ise2(socket, ise_id, OK);
	}
}*/
