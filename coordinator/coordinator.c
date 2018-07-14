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

int receive_sentence_execution_request(int ise_socket, t_sentence** sentence) {
	*sentence = malloc(sizeof(t_sentence)); //TODO acá hay un leak.
	int result;
	if ((result = recv_message(ise_socket)) != PROCESS_SENTENCE) {
		return result > 0? -1 : result;
	}

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

t_instance* select_instance_to_send_by_equitative_load(){
	t_instance* selected = NULL;

	pthread_mutex_lock(&instances_list_mtx);

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

	pthread_mutex_unlock(&instances_list_mtx);

	return selected;
}

bool is_instance_available(t_instance* instance){
		return instance -> is_available == true;
}

t_instance* select_instance_by_ke(char* key){

	int ascii_base_limit = 97; //a
	int ascii_final_limit = 122; //z
	int letters_qty = ascii_final_limit - ascii_base_limit;

	pthread_mutex_lock(&instances_list_mtx);
	t_list* available_instances = list_filter(instances_thread_list, (void*)is_instance_available);
	pthread_mutex_unlock(&instances_list_mtx);

	int instances_qty = available_instances -> elements_count;

	int ascii_key = tolower(key[0]);

	int keys_per_instance = ceil(letters_qty / instances_qty);

	int instance_index = floor((ascii_key - ascii_base_limit) / keys_per_instance);

	t_instance* selected_instance = (t_instance*) list_get(available_instances, instance_index);

	list_destroy(available_instances);

	return selected_instance;
}

t_instance* select_instance_to_send_by_lsu(){

	bool _has_less_entries_used_than(t_instance* instance, t_instance* other_instance){
		return (instance -> entries_in_use) < (other_instance -> entries_in_use);
	}

	//TODO es necesario tener semáforos para las instancias ? Si va a venir un ESI a la vez...
	t_list* available_instances = list_filter(instances_thread_list, (void*)is_instance_available);
	list_sort(available_instances, (void*) _has_less_entries_used_than);

	t_instance* selected_instance = list_get(available_instances, 0);

	if(selected_instance == NULL) {
		return NULL;
	}

	int entries_qty = selected_instance -> entries_in_use;
	bool _same_entries_used(t_instance* instance){
			return instance -> entries_in_use == entries_qty;
	}

	t_list* instances_with_same_entries_than_selected_instance = list_filter(available_instances, (void*) _same_entries_used);

	if(instances_with_same_entries_than_selected_instance -> elements_count > 1) {
		selected_instance = (t_instance*) select_instance_to_send_by_equitative_load();
	}

	list_destroy(available_instances);
	list_destroy(instances_with_same_entries_than_selected_instance);

	return selected_instance;
}

int health_check(t_instance* instance){
	int status = send(instance -> socket_id, &HEALTH_CHECK, sizeof(message_type), 0);
	if(status <= 0){
		log_error(logger, "Health check to instance %s. It will be marked as unavailable", instance -> name);
		handle_instance_disconnection(instance);
		return -1;
	}

	int health_check_result;
	int health_check_confirmation = recv(instance -> socket_id, &health_check_result, sizeof(int), 0);

	if(health_check_confirmation <= 0 || health_check_result != OK){
		log_error(logger, "Error receiving health check result from instance %s. It will be marked as unavailable", instance -> name);
		handle_instance_disconnection(instance);
		return -1;
	}

	return health_check_result;
}

t_instance* select_instance_to_send_by_distribution_strategy_and_operation(t_sentence* sentence) {
	t_instance* _look_for_instance(){
		switch(distribution) {
			case EL: return select_instance_to_send_by_equitative_load();
			case LSU: return select_instance_to_send_by_lsu();
			case KE: return select_instance_by_ke(sentence -> key);
			default: _exit_with_error(-1, "Invalid distribution strategy.", NULL);
			return NULL;
		}
	}

	pthread_mutex_lock(&keys_mtx);
	t_instance* instance = (t_instance*) dictionary_get(keys_location, sentence -> key);
	pthread_mutex_unlock(&keys_mtx);

	bool is_available = false;

	if(sentence -> operation_id == SET_SENTENCE && instance == NULL && is_available == false){
		while(instance == NULL || !is_available){
			instance = _look_for_instance();
			if(instance != NULL){
				int result = health_check(instance);
				is_available = (result == 1) ? true : false;
			}
		}
	}

	return instance;
}

void handle_instance_disconnection(t_instance* instance){
	pthread_mutex_lock(&instances_list_mtx);
	instance -> is_available = false;
	pthread_mutex_unlock(&instances_list_mtx);
	last_instance_selected -> is_available = false; //TODO poner semaforo acá.
	log_info(logger, "Instance %s has been marked as UNAVAILABLE", instance -> ip_port);
}

//TODO Hacer los free correspondientes!!!
int send_statement_to_instance_and_wait_for_result(t_instance* instance, t_sentence *sentence){
	log_info(logger, "Sending sentence to instance %s", instance -> name);

	t_buffer buffer = serialize_sentence(sentence);

	int send_result = send(instance -> socket_id, buffer.buffer_content, buffer.size, 0);
	destroy_buffer(buffer);

	if (send_result <= 0) {
		log_error(logger, "Could not send sentence operation id to instance.");
	}

	int result;
	int entries_used;
	int result_response;
	recv(instance -> socket_id, &result, sizeof(int), 0);
	result_response = recv(instance -> socket_id, &entries_used, sizeof(int), 0);

	instance -> entries_in_use = entries_used;

	if(result_response == 0) {
		log_error(logger, "Selected instance is not available !");
		handle_instance_disconnection(instance);
		return KEY_UNREACHABLE;
	}

	pthread_mutex_lock(&keys_mtx);
	dictionary_put_posta(keys_location, sentence -> key, instance);
	pthread_mutex_unlock(&keys_mtx);

	return result;
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
		pthread_mutex_unlock(&operations_log_file_mtx);
		return;
	}

	txt_write_in_file(operations_log_file, string_to_save);
	txt_close_file(operations_log_file);
	pthread_mutex_unlock(&operations_log_file_mtx);

	free(string_to_save);
	log_info(logger, "Operations log successfully saved");
}

void send_statement_result_to_ise(int socket, long ise_id, execution_result result) {
	int message_size = sizeof(message_type) + sizeof(int);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &EXECUTION_RESULT, sizeof(message_type));
	concat_value(&offset, &result, sizeof(int));
	int send_result = send(socket, buffer, message_size, 0);
	free(buffer);

	if (send_result <= 0) {
		log_error(logger, "Could not send sentence execution result to ESI %d", ise_id);
	}
}

void configure_logger() {
	logger = log_create("coordinator.log", "coordinator", 1, LOG_LEVEL_DEBUG);
}

void exit_gracefully(int code) {
	config_destroy(config);
	log_destroy(logger);
	free(instance_configuration);

	pthread_mutex_lock(&instances_list_mtx);
	void close_instance_connection(t_instance* instance) {
		close(instance->socket_id);
	}
	list_iterate(instances_thread_list, (void*) close_instance_connection);
	list_destroy(instances_thread_list);
	pthread_mutex_unlock(&instances_list_mtx);
	pthread_mutex_destroy(&instances_list_mtx);
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
	delay = config_get_int_value(config, "DELAY");

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

int send_instance_configuration(int client_sock, char *name){
	t_list *instance_keys = list_create(); //TODO leak
	int keys_size = 0;
	void _instance_keys_iterator(char *key, void *entry) {
		t_instance *instance = (t_instance *) entry;
		if (string_equals_ignore_case(instance->name, name)) {
			list_add(instance_keys, (void *)key);
			keys_size += strlen(key) + 1;
		}
	}
	dictionary_iterator(keys_location, _instance_keys_iterator);

	int keys_count = list_size(instance_keys);

	char* client_address = get_client_address(client_sock);
	log_info(logger, "Sending instance configuration to host %s. Keys count: %d", client_address, keys_count);
	free(client_address);

	int buffer_size = sizeof(t_instance_configuration) + sizeof(keys_size) + keys_size * sizeof(char) + keys_count * sizeof(int);
	void* buffer = malloc(buffer_size); //TODO leak
	void* offset = buffer;
	concat_value(&offset, instance_configuration, sizeof(t_instance_configuration));
	concat_value(&offset, &keys_count, sizeof(keys_count));
	void _concat_key(void *item) {
		char *key = (char *)item;
		concat_string(&offset, key, strlen(key) + 1);
		log_debug(logger, "Concat key %s", key);
	}
	list_iterate(instance_keys, _concat_key);


	int status = send(client_sock, buffer, buffer_size, 0);
	free(buffer);
	if(status <= 0){
		log_error(logger, "Could not send instance configuration.%d", status);
		close(client_sock);
		return 1;
	}
	log_info(logger, "Configuration successfully sent.");

	return 0;
}

void delay_execution(){
	sleep(delay * 0.001);
}

//TODO test cuando la instancia reciba bien la orden de compactar.
void start_compaction(){
	void _send_compaction_order(t_instance* instance){
		log_info(logger, "Sending compaction order to instance %s", instance -> name);

		int status = send(instance -> socket_id, &START_COMPACTION, sizeof(message_type), 0); //TODO testear sin el casteo.
		if(status <= 0){
			log_error(logger, "Error while sending compaction order to instance %s. It will be marked as unavailable", instance -> name);
			handle_instance_disconnection(instance);
		}
	}

	void _recv_compaction_result(t_instance* instance) {
		int compaction_result;
		int compaction_confirmation = recv(instance -> socket_id, &compaction_result, sizeof(int), 0);

		if(compaction_confirmation <= 0 || compaction_result != OK){
			log_error(logger, "Error receiving compaction result from instance %s. It will be marked as unavailable", instance -> name);
			handle_instance_disconnection(instance);
		}

		log_info(logger, "Compaction finished for instance %s", instance -> name);
	}

	log_info(logger, "Starting compaction process...");

	t_list* available_instances;
	available_instances = list_filter(instances_thread_list, (void*) is_instance_available);
	list_iterate(available_instances, (void*) _send_compaction_order);
	list_destroy(available_instances);

	available_instances = list_filter(instances_thread_list, (void*) is_instance_available);
	list_iterate(available_instances, (void*) _recv_compaction_result);
	list_destroy(available_instances);

	log_info(logger, "Compaction process finished.");
	return;
}

int process_sentence(t_sentence* sentence, long ise_id){
	int _send_to_instance(t_instance* selected_instance){
		free(last_instance_selected);
		last_instance_selected = malloc(sizeof(t_instance));
		memcpy(last_instance_selected, selected_instance, sizeof(t_instance));
		return send_statement_to_instance_and_wait_for_result(selected_instance, sentence);
	}

	int result_to_ise;
	t_instance* selected_instance;

	delay_execution();

	int planifier_validation = notify_sentence_and_ise_to_planifier(sentence -> operation_id, sentence -> key, ise_id);

	if(planifier_validation == OK){

		if((sentence -> operation_id) != GET_SENTENCE) {

			selected_instance = select_instance_to_send_by_distribution_strategy_and_operation(sentence);

			int send_to_instance_result;

			if(selected_instance != NULL){
				send_to_instance_result = _send_to_instance(selected_instance);
			} else {
				send_to_instance_result = KEY_UNREACHABLE;
			}
			//TODO agregar un log del resultado de la instancia.

			if(send_to_instance_result == NEED_COMPACTION){
				start_compaction();
				if(selected_instance -> is_available) {
					send_to_instance_result = _send_to_instance(selected_instance);
				} else {
					send_to_instance_result = KEY_UNREACHABLE;
				}
			}

			pthread_mutex_lock(&keys_mtx);
			if(send_to_instance_result == KEY_UNREACHABLE || selected_instance == NULL) {

				//if(sentence -> operation_id == STORE_SENTENCE){
				dictionary_remove(keys_location, sentence -> key);
				notify_sentence_and_ise_to_planifier(KEY_UNREACHABLE, sentence -> key, ise_id);
				result_to_ise = KEY_UNREACHABLE;
				//}
				//- Si es SET, podríamos ir a otra instancia, hay que validarlo...sino no pasa nada. lo único que también correspondiería avisarle al planif*/
			}
			dictionary_put_posta(keys_location, sentence -> key, selected_instance);
			result_to_ise = send_to_instance_result;
			pthread_mutex_unlock(&keys_mtx);
		} else {
			result_to_ise = planifier_validation;
		}
	} else {
		result_to_ise = planifier_validation;
	}

	return result_to_ise;
}

//TODO ver qué se puede reutilizar...cuando se envía la instrucción a la instancia hace algo parecido.
int notify_sentence_and_ise_to_planifier(int operation_id, char* key, long ise_id){
	log_info(logger, "Asking for sentence and resource to planifier %s", key);

	t_buffer buffer = serialize_operation_resource_request(operation_id, key, ise_id);

	int send_result = send(planifier_socket, buffer.buffer_content, buffer.size, 0);
	destroy_buffer(buffer);

	if (send_result <= 0) {
		_exit_with_error(planifier_socket, "Could not send sentence to planifier.", NULL); //TODO tener en cuenta que hay muchos sockets que cerrar si hay que bajar el coordinador !!
	}

	int result;

	//TODO AUXILIOOOOOOOO, QUÉ HAGO ACÁ ?
	if(recv_int(planifier_socket, &result) <= 0) {
		_exit_with_error(planifier_socket, "Could not receive resource response to planifier.", NULL);
	}

	return result;
	//return OK;
}

int main(int argc, char* argv[]) {
	instances_thread_list = list_create();
	ise_thread_list = list_create();
	configure_logger();
    signal(SIGINT,signal_handler);
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	pthread_mutex_t instances_mtx_aux = PTHREAD_MUTEX_INITIALIZER;
	instances_list_mtx = instances_mtx_aux;
	pthread_mutex_t keys_mtx_aux = PTHREAD_MUTEX_INITIALIZER;
	keys_mtx = keys_mtx_aux;
	pthread_mutex_t operations_log_file_mtx_aux = PTHREAD_MUTEX_INITIALIZER;
	operations_log_file_mtx = operations_log_file_mtx_aux;
	OPERATIONS_LOG_PATH = "operations.log";

	int server_socket = start_server(server_port, server_max_connections, (void *)connection_handler, false, logger);
	check_server_startup(server_socket); //TODO llevar esto adentro del start_server ?

	//**PARA TEST***/
	/*while(instances_thread_list -> elements_count < 3);

	srand(time(NULL));

	t_sentence* sentence1 = sentence_create_with(GET_SENTENCE, "barcelona:jugadores", "messi");
	process_sentence(sentence1, 1);
	t_sentence* sentence2 = sentence_create_with(SET_SENTENCE, "barcelona:jugadores", "messi");
	process_sentence(sentence2, 1);
	t_sentence* sentence3 = sentence_create_with(STORE_SENTENCE, "barcelona:jugadores", "messi");
	process_sentence(sentence3, 1);

	t_sentence* sentence4 = sentence_create_with(GET_SENTENCE, "independiente:jugadores", "meza");
	process_sentence(sentence4, 3);
	t_sentence* sentence5 = sentence_create_with(SET_SENTENCE, "independiente:jugadores", "meza");
	process_sentence(sentence5, 3);
	t_sentence* sentence6 = sentence_create_with(STORE_SENTENCE, "independiente:jugadores", "meza");
	process_sentence(sentence6, 3);

	t_sentence* sentence7 = sentence_create_with(GET_SENTENCE, "sanmartindetucuman:jugadores", "busse");
	process_sentence(sentence7, 2);
	t_sentence* sentence8 = sentence_create_with(SET_SENTENCE, "sanmartindetucuman:jugadores", "busse");
	process_sentence(sentence8, 2);
	t_sentence* sentence9 = sentence_create_with(STORE_SENTENCE, "sanmartindetucuman:jugadores", "busse");
	process_sentence(sentence9, 2);

	t_sentence* sentence10 = sentence_create_with(GET_SENTENCE, "independiente:jugadores", "gigliotti");
	process_sentence(sentence10, 2);
	t_sentence* sentence11 = sentence_create_with(SET_SENTENCE, "independiente:jugadores", "gigliotti");
	process_sentence(sentence11, 2);
	t_sentence* sentence12 = sentence_create_with(STORE_SENTENCE, "independiente:jugadores", "gigliotti");
	process_sentence(sentence12, 2);

	t_sentence* sentence13 = sentence_create_with(GET_SENTENCE, "argentina:jugadores", "tagliafico");
	process_sentence(sentence13, 2);
	t_sentence* sentence14 = sentence_create_with(SET_SENTENCE, "argentina:jugadores", "tagliafico");
	process_sentence(sentence14, 2);
	t_sentence* sentence15 = sentence_create_with(STORE_SENTENCE, "argentina:jugadores", "tagliafico");
	process_sentence(sentence15, 2);*/

	exit_gracefully(EXIT_SUCCESS);
}
