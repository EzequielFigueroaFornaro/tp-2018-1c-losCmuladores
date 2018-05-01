/*
 ============================================================================
 Name        : ise.c
 Author      : losCmuladores
 ============================================================================
 */

#include "ise.h"

int send_operation(t_esi_operacion sentence);

int main(int argc, char* argv[]) {
	configure_logger();
	log_info(logger, "Initializing...");
	load_configuration(argv[1]);

	handshake_coordinator();
	handshake_planifier();

	t_ise_script * script = load_script(argv[2]);
	wait_to_execute();
	execute_script(script);
	return EXIT_SUCCESS;
}

int handshake_coordinator() {
	log_info(logger, "Starting handshake process with coordinator...");
	int connect_result = connect_to(coordinator_ip, coordinator_port);
	if (connect_result < 0) {
		log_error(logger, "Could not connect to coordinator on %s:%d",
				coordinator_ip, coordinator_port);
		exit_gracefully(EXIT_FAILURE);
	}

	int socket = connect_result;

	if (send_module_connected(socket, ISE) < 0) {
		log_error(logger, "Could not send message to coordinator on %s:%d", coordinator_ip, coordinator_port);
		close(socket);
		exit_gracefully(EXIT_FAILURE);
	}

	message_type message_type;
	int message_type_result = recv(socket, &message_type, sizeof(message_type),	MSG_WAITALL);
	if (message_type_result <= 0 || message_type != CONNECTION_SUCCESS) {
		log_error(logger,
				"Expected CONNECTION_SUCCESS (%d) from coordinator on %s:%d, actual: %d",
				CONNECTION_SUCCESS, coordinator_ip, coordinator_port, message_type);
		close(socket);
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(logger, "Connected to coordinator!");
	return connect_result;
}

int handshake_planifier() {
	log_info(logger, "Starting handshake process with planifier...");
	int connect_result = connect_to(planifier_ip, planifier_port);
	if (connect_result < 0) {
		log_error(logger, "Could not connect to planifier on %s:%d",
				planifier_ip, planifier_port);
		exit_gracefully(EXIT_FAILURE);
	}

	int socket = connect_result;
	if (send_module_connected(socket, ISE) < 0) {
		log_error(logger, "Could not send message to planifier on %s:%d", planifier_ip, planifier_port);
		close(socket);
		exit_gracefully(EXIT_FAILURE);
	}

	message_type message_type;
	int message_type_result = recv(socket, &message_type, sizeof(message_type),	MSG_WAITALL);

	if (message_type_result <= 0 || message_type != CONNECTION_SUCCESS) {
		log_error(logger,
				"Expected CONNECTION_SUCCESS (%d) from planifier on %s:%d, actual: %d",
				CONNECTION_SUCCESS, planifier_ip, planifier_port, message_type);
		close(socket);
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(logger, "Connected to planifier!");
	return connect_result;
}

t_ise_script * load_script(char * script_file_name) {
	FILE * script_file = fopen(script_file_name, "r");

	if (script_file == NULL) {
		log_error(logger, "File %s not found", script_file_name);
		exit_gracefully(1);
	}

	t_ise_script * script = malloc(sizeof (t_ise_script));
	script->instructions = queue_create();
	while (!(feof(script_file))) {
		size_t line_size = 0;
		char * line;
		bool valid_line = getline(&line, &line_size, script_file) != -1;
		if (valid_line) {
			queue_push(script->instructions, line);
		}
	}
	fclose(script_file);
	return script;
}

void wait_to_execute() {
}

void execute_script(t_ise_script * script) {
	char * instruction;
	while ((instruction = queue_pop(script->instructions)) != NULL) {
		string_trim(&instruction);
		t_esi_operacion sentence = parse(instruction);
		int result = send_operation(sentence);
		if (result != 0) {
			log_error(logger, "Error trying to send sentence '%s' to coordinator", instruction);
			destroy_script(script);
			exit_gracefully(1);
		}
		log_info(logger, "Sentence '%s' sent successfully", instruction);
		notify_execution_success();
	}
}

int send_operation(t_esi_operacion sentence) {
	return 0;
}

void notify_execution_success() {
}

void exit_gracefully(int code) {
	log_destroy(logger);
	exit(code);
}

void print_script(t_ise_script * script) {
	for (int i = 0; i < queue_size(script->instructions); i++) {
		char* instruction = list_get(script->instructions->elements, i);
		printf("%s", instruction);
	}
}

void configure_logger() {
	logger = log_create("ise.log", "ise", 1, LOG_LEVEL_INFO);
}

void load_configuration(char* config_file_path) {
	configure_logger();
	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	planifier_port = config_get_int_value(config, "PLANIFIER_PORT");
	planifier_ip = config_get_string_value(config, "PLANIFIER_IP");

	coordinator_port = config_get_int_value(config, "COORDINATOR_PORT");
	coordinator_ip = config_get_string_value(config, "COORDINATOR_IP");

	log_info(logger, "OK Loading.");
}

void destroy_script(t_ise_script * script) {
	queue_destroy(script->instructions);
	free(script);
}
