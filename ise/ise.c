/*
 ============================================================================
 Name        : ise.c
 Author      : losCmuladores
 ============================================================================
 */

#include "ise.h"

void load_script(char* file_name);
void exit_with_error();

int main(int argc, char* argv[]) {
	init_logger();
	load_configuration(argv[1]);
	load_script(argv[2]);

	connect_to_planifier();
	connect_to_coordinator();

	execute_script();

	exit_gracefully(EXIT_SUCCESS);
}

void notify_error() {
	// TODO [Lu]
}

void execute_script() {
	t_ise_sentence sentence;

	while (!(sentence = next_sentence()).empty) {
		wait_to_execute();

		t_sentence_process_result result;
		if (sentence.operation.valido) {
			result = send_operation(sentence.operation);
		} else {
			result = INVALID_SENTENCE;
		}

		if (result != OK) {
			notify_error(); // TODO [Lu] Revisar si esto va
			exit_with_error();
		}
		execution_signal = notify();
	}
}

void wait_to_execute() {
	message_type execution_signal = recv_message(planifier_socket);
	while (execution_signal != ISE_EXECUTE);
}

message_type notify() {
	message_type message_type = SENTENCE_EXECUTED_OK;
	if (send(planifier_socket, &message_type, sizeof(message_type), 0) < 0) {
		log_error(logger, "Could not notify planifier of sentence execution");
	}
	return recv_message(planifier_socket);
}

t_sentence_process_result send_operation(t_esi_operacion operation) {
	// TODO [Lu]
	return OK;
}

void load_script(char* file_name) {
	int load_result = load_script_from_file(file_name);
	if (load_result != 0) {
		log_error(logger, "Could not load script from file %s.", file_name);
		exit_with_error();
	}
}

void exit_gracefully(int code) {
	close(planifier_socket);
	close(coordinator_socket);
	log_destroy(logger);
	destroy_script(script);
	exit(code);
}

void exit_with_error() {
	exit_gracefully(EXIT_FAILURE);
}

void load_configuration(char* config_file_path) {
	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	planifier_port = config_get_int_value(config, "PLANIFIER_PORT");
	planifier_ip = config_get_string_value(config, "PLANIFIER_IP");

	coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	coordinator_ip = config_get_string_value(config, "COORDINATOR_IP");

	log_info(logger, "OK Loading.");
}

/* TODO [Lu] Lógica repetida */
/**********************************************************/
void connect_to_planifier() {
	planifier_socket = connect_to(planifier_ip, planifier_port);
	if (planifier_socket < 0) {
		log_error(logger, /* TODO [Lu] loggear errno*/"Could not connect to planifier");
		exit_with_error();
	}

	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(long);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	offset = concat_value(offset, &MODULE_CONNECTED, sizeof(message_type));
	offset = concat_value(offset, &ISE, sizeof(module_type));
	long script_size = queue_size(script->lines);
	concat_value(offset, &script_size, sizeof(script_size));

	int send_result = send(coordinator_socket, buffer, message_size, 0);
	free(buffer);
	if (send_result < 0) {
		log_error(logger, "Could send handshake to planifier");
		exit_with_error();
	}

	if (recv_message(coordinator_socket) != CONNECTION_SUCCESS) {
		log_error(logger, "Planifier rejected handshake");
		exit_with_error();
	}

	recv_string(planifier_socket, &my_id);
}

void connect_to_coordinator() {
	coordinator_socket = connect_to(coordinator_ip, coordinator_port);
	if (coordinator_socket < 0) {
		log_error(logger, /* TODO [Lu] loggear errno*/"Could not connect to coordinator");
		exit_with_error();
	}

	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(long) + strlen(my_id);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	offset = concat_value(offset, &MODULE_CONNECTED, sizeof(message_type));
	offset = concat_value(offset, &ISE, sizeof(module_type));
	concat_string(offset, &my_id, strlen(my_id));

	int send_result = send(coordinator_socket, buffer, message_size, 0);
	free(buffer);
	if (send_result < 0) {
		log_error(logger, "Could send handshake to coordinator");
		exit_with_error();
	}

	if (recv_message(coordinator_socket) != CONNECTION_SUCCESS) {
		log_error(logger, "Coordinator rejected handshake");
		exit_with_error();
	}
}
/**********************************************************/
