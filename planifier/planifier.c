/*
 ============================================================================
 Name        : planifier.c
 Author      : losCmuladores
 Version     :
 Copyright   : Your copyright notice
 Description : Redistinto planifier	.
 ============================================================================
 */

#include "planifier.h"

int main(int argc, char* argv[]) {
	e = dictionary_create();

	set_orchestrator();
	configure_logger();
	load_configuration(argv[1]);
	connect_to_coordinator();
	int server_started = start_server(server_port, server_max_connections, (void *) esi_connection_handler, true, logger);
	if (server_started < 0) {
		log_error(logger, "Server not started");
	}

	pthread_t console_thread = start_console();

	pthread_join(console_thread, NULL);
	return EXIT_SUCCESS;
}

int esi_id_generate(){
	pthread_mutex_trylock(&id_mtx);
	int new_id = id ++;
	pthread_mutex_unlock(&id_mtx);
	return new_id;
}
int cpu_time_incrementate(){
	pthread_mutex_trylock(&cpu_time_mtx);
	int new_cpu_time = cpu_time ++;
	pthread_mutex_unlock(&cpu_time_mtx);
	return new_cpu_time;
}
int new_esi(/*me da algo*/){
	pthread_mutex_trylock(&cpu_time_mtx);
	esi new_esi = malloc(sizeof(esi));
	new_esi -> id = esi_id_generate();
	pthread_mutex_trylock(&cpu_time_mtx);
	new_esi -> tiempo_de_entrada = cpu_time;
	pthread_mutex_unlock(&cpu_time_mtx);
	new_esi -> cantidad_de_instrucciones = 123/*lo tiene que pasar por parametro el esi, creo que s lo unico que te tiene que pasar*/;
	new_esi -> instrucction_pointer = 0;
	pthread_mutex_unlock(&cpu_time_mtx);
	add_esi(new_esi);
	return new_esi -> id;
}

// funciones a modificar o hacer

//new_esi(que tiene solo input)
void new_esi(esi* new_esi){
	new_esi -> id = esi_id_generate();
	add_esi(new_esi);
}

voy_a_ejecutar(tiene input output)
//bool hubo_replanificacion_con_cambio_de_esi
///*este bool lo tiene que tener por referencia los algoritmos
// * y poder modificarlo cada vez que hay replanificacion usando semaforos*/
//che_ejecute_esto(int esi_id){
/*me da algo, tendria que ser el id del esi, y me podria decir que es la ultima instruccion*/
//	if(hubo_replanificacion_con_cambio_de_esi){
//
//	}else{
//
//	}
}
//
//ejecuta_vos(output)

//che_esta_tomado_el_recurso(input_outpu)
bool bloquear_recurso(char* recurso, int esi_id){
	pthread_mutex_lock(&map_boqueados);
	if(!dictionary_has_key(recursos_bloqueados,recurso)){
		dictionary_put(recursos_bloqueados, recurso, queue_create());
		pthread_mutex_unlock(&map_boqueados);
		return true;
	}
	t_queue* cola_de_esis = dictionary_get(recursos_bloqueados,recurso);
	esi running_esi = get_esi_running();
	queue_push(cola_de_esis,(running_esi->id));
	stop_and_block_esi(running_esi -> id);
	pthread_mutex_unlock(&map_boqueados);
	return false;
}


//esi* get_esi_running(){
//	return list_get(RUNNING_ESI_LIST, 0);
//}

void liberar_recurso(char* recurso){
	pthread_mutex_lock(&map_boqueados);
	t_queue* esi_queue = dictionary_remove(recursos_bloqueados,recurso);
	int* esi_id = queue_push(esi_queue);
	pthread_mutex_unlock(&map_boqueados);
	add_esi_bloqueada(esi_id);
	//OJO AL PIJO el frre de datos como el id que guardamos de la esi bloqueada;
}



//-------------------
void configure_logger() {
	logger = log_create("planifier.log", "planifier", true, LOG_LEVEL_INFO);
}

void load_configuration(char *config_file_path) {
	log_info(logger, "Loading planifier configuration file...");
	t_config* config = config_create(config_file_path);

	algorithm = config_get_int_value(config, "ALGORITHM");

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
