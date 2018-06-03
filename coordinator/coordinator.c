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

//Recibe solicitud del ESI.
void receive_statement_request();

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

//TODO Hacer los free correspondientes!!!
int send_statement_to_instance_and_wait_for_result(t_instance* instance, t_sentence *sentence){
	//Antes de hacer esto, guardar en la tabla correspondiente en qué instancia quedó esta key...
	log_info(logger, "Sending sentence to instance %s", instance -> name);

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

//TODO llevar a commons.
char* get_operation_as_string(int operation_id){
	switch(operation_id) {
		case GET_SENTENCE: return "GET";
		case SET_SENTENCE: return "SET";
		case STORE_SENTENCE: return "STORE";
		default: return NULL;
	}
}

void save_operation_log(t_sentence* sentence, t_ise* ise){
	char* string_to_save = string_new();

	char* operation = get_operation_as_string(sentence -> operation_id);

	string_append(&string_to_save, "ESI");
	string_append_with_format(&string_to_save, "%d | ", ise -> id);
	string_append_with_format(&string_to_save, "%s | ", operation);
	string_append_with_format(&string_to_save, "%s", sentence -> key);
	string_append(&string_to_save, "\n");
	if(sentence -> value != NULL){
		string_append(&string_to_save, sentence -> value);
	}

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

//Devuelve el resultado al ESI.
void send_statement_result_to_ise(int result, int socket);

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

//TODO cerrar TODOS los sockets (planificador, el parametrizado, y el de todas las instancias conectadas)
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

//TODO ver qué se puede reutilizar...cuando se envía la instrucción a la instancia hace algo parecido.
int send_sentence_to_planifier_and_receive_confirmation(t_sentence* sentence){
	t_buffer buffer = serialize_sentence(sentence);

	int send_result = send(planifier_socket, buffer.buffer_content, buffer.size, 0);
	destroy_buffer(buffer);

	if (send_result <= 0) {
		_exit_with_error(planifier_socket, "Could not send sentence to planifier.", NULL); //TODO tener en cuenta que hay muchos sockets que cerrar si hay que bajar el coordinador !!
	}

	int result;
	int result_response = recv(planifier_socket, &result, sizeof(int), 0);

	//TODO AUXILIOOOOOOOO, QUÉ HAGO ACÁ ?
	if(result_response <= 0) {
		_exit_with_error(planifier_socket, "Could not receive resource response to planifier.", NULL);
	}

	//TODO asumo que se mantiene el protocolo en todo el sistema.
	return result;
}


int can_resource_be_used(t_sentence* sentence){

	log_info(logger, "Asking for sentence and resource to planifier %s");

	if(sentence -> operation_id == GET_SENTENCE) {
		//checkear existencia de la key. Si no existe crearla
	}

	//Avisar/Preguntarle al planificador de esta instruccion con este recurso.

	//TODO todavía no está integrado el planificador.
	//return send_sentence_to_planifier_and_receive_confirmation(sentence);

	return 0;
}

/*
 * case OK : return "Sentencia ejecutada"; 0
 * 	case KEY_TOO_LONG : return "Error de Tamano de Clave"; 1
 * 	case KEY_NOT_FOUND : return "Error de Clave no Identificada"; 2
 * 	case KEY_UNREACHABLE : return "Error de Clave Inaccesible"; 3
 * 	case KEY_LOCK_NOT_ACQUIRED : return "Error de Clave no Bloqueada"; 4
 * 	case KEY_BLOCKED : return "Clave bloqueada por otro proceso"; 5
 * 	case PARSE_ERROR : return "Error al intentar parsear sentencia"; 6
 * */

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


	int resource_can_be_manipulated = can_resource_be_used(sentence);

	if(resource_can_be_manipulated == 0) { //OK.

		t_instance* selected_instance = select_instance_to_send_by_distribution_strategy(forced_key[0]);

		int resullt = send_statement_to_instance_and_wait_for_result(selected_instance, sentence);

		if(send_statement_to_instance_and_wait_for_result(selected_instance, sentence) == -1) {
			if(sentence -> operation_id == GET_SENTENCE){

			}
			/*Lalala, acá deberíamos ver qué es lo que se está tratando de hacer...
			 - Si es GET, vamos a otra instancia no pasa nada
			 - Si es SET o STORE, tendríamos que avisar al planificador para qeu aborte el ESI correspondiente.*/

		}
		save_operation_log(sentence, ise);

		//TODO mapear errores del resultado de la las instancias, al protocolo de ESI.
		//send_statement_result_to_ise(resullt);

	} else {

	}


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
