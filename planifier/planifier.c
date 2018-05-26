/*
 ============================================================================
 Name        : planifier.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto planifier.
 ============================================================================
 */

#include "planifier.h"

int main(int argc, char* argv[]) {

	ready_esi_list = list_create();
	running_esi_list = list_create();
	blocked_esi_list = list_create();
	finished_esi_list = list_create();
	recursos_bloqueados = dictionary_create();

	configure_logger();
	load_configuration(argv[1]);

	connect_to_coordinator();

	int server_started = start_server(server_port, server_max_connections, (void *) esi_connection_handler, true, logger);
	if (server_started < 0) {
		log_error(logger, "Server not started");
	}

	pthread_t console_thread = start_console();

	/* a - tenemosq que hace un lisener que escuche los nuevos esis
	 *
	 * b - tenemos que manear una lista de recusrsos tomados por distintos esis
	 *
	 * c - tenmos que hacer que el algoritmo barie por configuracion
	 *
	 * d - manejar las colas de estados de los esis, bloqueados , listos , ejecutando
	 *
	 *
	 *
	 * a - cada vez que llegue un esi, hay que replanificar, hay que manejar una flag para no tomar instrucciones cunado se esta replanificando o vizebersa
	 *
	 * b - tenmosq ue ver quien nos informa de que un recurso fue liberado
	 *
	 * c -
	 *
	 * b -
	 *
	 * * cuando el esi nos manda la respuesta del cordinador, tenemos posibles respuestas : un codigo que signifique segui corrientes, y otro que sea una pausa porque error
	 *
	 *
	 *
	 * */

	pthread_join(console_thread, NULL);

	return EXIT_SUCCESS;
}

// funciones a modificar o hacer

int id = 0;
int esi_id_generate(){
	pthread_mutex_trylock(&id_mtx);
	int new_id = id ++;
	pthread_mutex_unlock(&id_mtx);
	return new_id;
}


void new_esi(){}

bool bloquear_recurso(char* recurso){
	pthread_mutex_lock(&map_boqueados);
	if(!dictionary_has_key(recursos_bloqueados,recurso)){
		dictionary_put(recursos_bloqueados,recurso,list_create());
		pthread_mutex_unlock(&map_boqueados);
		return true;
	}
	t_list * listas_de_esis = dictionary_get(recursos_bloqueados,recurso);
	//TODO aca vamos a buscar el esi que se esta ejecutando y los vamos aagragar la la lista
	//list_add(listas_de_esis,esi);
	pthread_mutex_unlock(&id_mtx);
	return false;
}


void liberar_recurso(char* recurso){
	pthread_mutex_lock(&map_boqueados);
	t_list * listas_de_esis = dictionary_remove(recursos_bloqueados,recurso);
	pthread_mutex_unlock(&map_boqueados);
//	desbloquea_esis(listas_de_esis);
}

void ejecutar_esi(){
	esi* esi = malloc(sizeof(esi));
	get_more_priority_esi(ready_esi_list, esi);
	put_on_list(running_esi_list, esi, &running_esi_sem_list);
//	mandar_a_correr(esi);
	free(esi);
} //mandar mensaje de que se ejecute y poner en lista de ejecutados


void bloquea_esi(){} // sacar de lista poner en lista de bloqueados



void desbloquea_esis(t_list* esis_id_liberadas){
//	bool es_un_esi_libre(esi* esi) {
//		int size = list_size(esis_id_liberadas*);
//		for (int i = 0; i < size; ++i) {
//			if (list_get_element(list,i)==(esi->id)) {
//				return true;
//			}
//		}
//		return false;
//	}
//
//	t_list esi_liberadas = list_filter_and_remove(blocked_esi_list,(void*) es_un_esi_libre);
//	list_add_all(ready_esi_list, esi_liberadas);

} // sacar de la lista de bloqueados y poner en rdy


void tomar_respuesta(){} // el esi te informa lo que el cordinador le respondio


void put_on_list(t_list* list ,esi* esi, pthread_mutex_t sem_list){
	pthread_mutex_lock(&sem_list);
	list_add(list, esi);
	pthread_mutex_unlock(&sem_list);
}

void get_more_priority_esi(t_list * list, esi* esi){
	pthread_mutex_trylock(&ready_esi_sem_list);
	esi = list_remove(list, 0);
	pthread_mutex_unlock(&ready_esi_sem_list);
}

void configure_logger() {
	logger = log_create("planifier.log", "planifier", true, LOG_LEVEL_INFO);
}

void load_configuration(char *config_file_path) {
	log_info(logger, "Loading planifier configuration file...");
	t_config* config = config_create(config_file_path);

	server_port = config_get_int_value(config, "SERVER_PORT");
	server_max_connections = config_get_int_value(config,
			"MAX_ACCEPTED_CONNECTIONS");

	coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	coordinator_ip = string_duplicate(config_get_string_value(config, "COORDINATOR_IP"));

	config_destroy(config);

	log_info(logger, "Planifier configuration file loaded");
}

void connect_to_coordinator() {
	coordinator_socket = connect_to(coordinator_ip, coordinator_port);

	if (coordinator_socket < 0) {
		exit_with_error(coordinator_socket, "No se pudo conectar al coordinador");
	} else if (send_module_connected(coordinator_socket, PLANIFIER) < 0) {
		exit_with_error(coordinator_socket, "No se pudo enviar al confirmacion al coordinador");
	} else {
		message_type message_type;
		int message_type_result = recv(coordinator_socket, &message_type,
				sizeof(message_type), MSG_WAITALL);

		if (message_type_result < 0 || message_type != CONNECTION_SUCCESS) {
			exit_with_error(coordinator_socket, "Error al recibir confirmacion del coordinador");
		} else {
			log_info(logger, "Connexion con el coordinador establecida");
		}
	}
}

void send_coordinator_connection_completed(int coordinator_socket) {
	if (send_connection_success(coordinator_socket) < 0) {
		exit_with_error(coordinator_socket,
				"Error sending coordinator completed");
	} else {
		log_info(logger, "Coordinator connection completed");
	}
}

void esi_connection_handler(int socket) {
	message_type message_type;
	int result_message_type = recv(socket, &message_type, sizeof(message_type), MSG_WAITALL);

	if (result_message_type <= 0) {
		log_error(logger, "Error trying to receive message. Closing connection");
		close(socket);
		return;
	}

	if (message_type != MODULE_CONNECTED) {
		log_warning(logger, "Connection was received but the message type does not imply connection. Ignoring");
		close(socket);
		return;
	}

	module_type module_type;
	int result_module_type = recv(socket, &module_type, sizeof(module_type),
			MSG_WAITALL);
	if (result_module_type <= 0) {
		log_error(logger, "Error trying to receive module type. Closing connection");
		close(socket);
		return;
	}

	if (module_type == ISE) {
		if (send_connection_success(socket) < 0) {
			log_error(logger, "Error sending \"connection success\" message to %s",	get_client_address(socket));
			close(socket);
			return;
		}
		log_info(logger, "ESI connected! (from %s)", get_client_address(socket));
	} else {
		log_info(logger, "Ignoring connected client because it was not an ESI");
	}
}

void listen_for_commands() {
	char *command;
	int is_exit_command;
	do {
		command = readline("Command: ");
		is_exit_command = string_equals_ignore_case(command, "EXIT");
		if (is_exit_command) {
			log_info(logger, "Exiting...");
		} else {
			log_info(logger, "Invalid command");
		}
		free(command);
	} while (!is_exit_command);
}

pthread_t start_console() {
	pthread_t console_thread;
	if (pthread_create(&console_thread, NULL, (void*) listen_for_commands, NULL) < 0) {
		exit_with_error(0, "Error starting console thread");
	};
	return console_thread;
}

void exit_gracefully(int return_nr) {
	log_destroy(logger);
	exit(return_nr);
}

void exit_with_error(int socket, char* error_msg) {
	log_error(logger, error_msg);
	close(socket);
	exit_gracefully(1);
}
