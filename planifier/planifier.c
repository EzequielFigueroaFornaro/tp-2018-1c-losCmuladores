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

long esi_id_generate(){
	pthread_mutex_trylock(&id_mtx);
	int new_id = id ++;
	pthread_mutex_unlock(&id_mtx);
	return new_id;
}

long cpu_time_incrementate(){
	pthread_mutex_trylock(&cpu_time_mtx);
	int new_cpu_time = cpu_time ++;
	pthread_mutex_unlock(&cpu_time_mtx);
	return new_cpu_time;
}

int new_esi(int socket, long esi_size){
	esi* new_esi = malloc(sizeof(esi));
	new_esi -> id = esi_id_generate();
	new_esi -> estado = NUEVO;
	new_esi -> tiempo_de_entrada = cpu_time;
	new_esi -> socket_id = socket;
	new_esi -> esi_thread = pthread_self();
	pthread_mutex_unlock(&cpu_time_mtx);
	new_esi -> cantidad_de_instrucciones = esi_size;
	new_esi -> instrucction_pointer = 0;
	pthread_mutex_unlock(&cpu_time_mtx);
	add_esi(new_esi);
	return new_esi -> id;
}

//che_ejecute_esto(tiene input output){ //esi diciendo ejecute algo
//bool hubo_replanificacion_con_cambio_de_esi
///*este bool lo tiene que tener por referencia los algoritmos
// * y poder modificarlo cada vez que hay replanificacion usando semaforos*/
//che_ejecute_esto(int esi_id){
/*me da algo, tendria que ser el id del esi, y me podria decir que es la ultima instruccion*/
//	if(hubo_replanificacion_con_cambio_de_esi){
//
//	}else{
//
//}
//}
//

int send_message_to_esi(long esi_id, message_type message){
	esi *esi_to_notify = malloc(sizeof(esi));
	esi_to_notify = dictionary_get(&esi_map, string_key(esi_id));
	int socket_id = esi_to_notify->socket_id;
	return send(socket_id, message, sizeof(message_type), 0);
}

void send_esi_to_run(long esi_id){
	if (send_message_to_esi(esi_id, ISE_EXECUTE) < 0){
		log_error(logger, "Could not send ise %l to run", esi_id);
		//todo que pasa si no le puedo mandar un mensaje?
	}
}

void send_esi_to_stop(long esi_id){
	if (send_message_to_esi(esi_id, ISE_STOP) < 0){
		log_error(logger, "Could not send ise %l to run", esi_id);
		//todo que pasa si no le puedo mandar un mensaje?
	}
}

//che_esta_tomado_el_recurso(input_outpu)

bool bloquear_recurso(char* recurso, long esi_id){
	pthread_mutex_lock(&map_boqueados);
	if(!dictionary_has_key(recurso_tomado_por_esi,recurso)){
		dictionary_put(recurso_tomado_por_esi, recurso, queue_create());
		pthread_mutex_unlock(&map_boqueados);
		return true;
	}
	t_queue* cola_de_esis = dictionary_get(recurso_tomado_por_esi,recurso);
	esi *running_esi = malloc(sizeof(esi));
//	running_esi = get_esi_running();
//	queue_push(cola_de_esis,(running_esi->id));
//	stop_and_block_esi(running_esi -> id);
	pthread_mutex_unlock(&map_boqueados);
	return false;
}

bool deshabilitar_recurso(char/*no se que es esto*/ recurso, long esi_id_desabilitado){
	pthread_mutex_lock(&map_boqueados);
	if(!dictionary_has_key(recurso_tomado_por_esi,recurso)){
		dictionary_put(recurso_tomado_por_esi, recurso, ESI_BLOQUEADO);
		dictionary_put(esis_bloqueados_por_recurso, recurso, queue_create());
		pthread_mutex_unlock(&map_boqueados);
		return true;
	}
	int esi_id = dictionary_get(recurso_tomado_por_esi,recurso);
	if(!(esi_id == esi_id_desabilitado)){
		t_queue* cola_de_esis = dictionary_get(esis_bloqueados_por_recurso,recurso);
		queue_push(cola_de_esis, esi_id);
//		stop_and_block_esi(esi_id);
	}
	finish_esi(esi_id);
	dictionary_put(recurso_tomado_por_esi, recurso, ESI_BLOQUEADO);
	pthread_mutex_unlock(&map_boqueados);
}


//esi* get_esi_running(){
//	return list_get(RUNNING_ESI_LIST, 0);
//}

void liberar_recurso(char* recurso){
	pthread_mutex_lock(&map_boqueados);
	//TODO ver que onda desbloqueo todo o una sola
	dictionary_remove(recurso_tomado_por_esi, recurso);
	t_queue* esi_queue = dictionary_get(esis_bloqueados_por_recurso,recurso);
	long esi_id = queue_pop(esi_queue);
	while(!is_valid_esi(esi_id)){
		esi_id = queue_pop(esi_queue);
	}
	pthread_mutex_unlock(&map_boqueados);
	add_esi_bloqueada(esi_id);
	//OJO AL PIJO el frre de datos como el id que guardamos de la esi bloqueada;
}



//-------------------
void configure_logger() {
	logger = log_create("planifier.log", "planifier", false, LOG_LEVEL_INFO);
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

		int operation;
		while(recv_sentence_operation(coordinator_socket, &operation) > 0){//todo que condicion pongo aca?
			switch(operation){
				case GET_SENTENCE:
					//esi_tomando_recurso_handler();
					break;
				case SET_SENTENCE:
					//esi_queriendo_otra_cosa_con_recurso_pero_debe_tenerlo_tomado_handler();
					break;
				case STORE_SENTENCE:
					//liberar_recurso_handler();
					break;
				default:
					log_info(logger, "Connection was received but the operation its not supported. Ignoring");
					break;
			}
		}
	}
}

void connection_handler(int socket) {
	message_type message_type_result = recv_message(socket);
	if(message_type_result == MODULE_CONNECTED){
		esi_connection_handler(socket);
	}else if(message_type_result == EXECUTION_RESULT){
		//execution_result_handler();
	}else{
		log_info(logger, "Connection was received but the message type does not imply connection or any operation. Ignoring");
		close(socket);
	}
}

void esi_connection_handler(int socket){
	module_type *module_type;
	int result_module_type = recv(socket, &module_type, sizeof(module_type), MSG_WAITALL);
	if (result_module_type <= 0) {
		log_error(logger, "Error trying to receive module type. Closing connection");
		close(socket);
		return;
	}

	if (module_type == ISE) {
		log_info(logger, "ESI connected! (from %s)", get_client_address(socket));

		long esi_size;
		int result_esi_size = recv_long(socket, &esi_size);

		if (result_esi_size <= 0) {
			log_error(logger, "Error trying to receive message. Closing connection");
			close(socket);
			return;
		}

		int new_esi_id = add_esi(socket, esi_size);
		int result_send_new_esi_id = send(socket, new_esi_id, sizeof(new_esi_id), 0);
		if (result_send_new_esi_id < 0) {
			log_error(logger, "Error sending the id to the new esi. Client-Address %s",	get_client_address(socket));
			close(socket);
			return;
		}
	} else {
		log_info(logger, "Ignoring connected client because it was not an ESI");
	}

}

void exit_gracefully(int return_nr) {
	log_destroy(logger);
	exit(return_nr);
}

void exit_with_error(int socket, char* error_msg) {
	log_error(logger, error_msg);
	print_error(error_msg);
	close(socket);
	exit_gracefully(1);
}

int main(int argc, char* argv[]) {

	esis_bloqueados_por_recurso = dictionary_create();
	int i = 1;
	set_orchestrator(i);
	configure_logger();
	load_configuration(argv[1]);
	int server_started = start_server(server_port, server_max_connections, (void *) connection_handler, true, logger);
	if (server_started < 0) {
		log_error(logger, "Server not started");
	}

	pthread_t console_thread = start_console();
	connect_to_coordinator();
	pthread_join(console_thread, NULL);
	return EXIT_SUCCESS;
}
