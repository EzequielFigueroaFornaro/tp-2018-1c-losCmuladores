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
	if (!init_connections()) {
		exit_with_error();
	}
	init_execution_signals_receiver();
	execute_script();

	exit_gracefully(EXIT_SUCCESS);
}

void init_execution_signals_receiver() {
	// TODO [LU]
}

void notify_sentence_error(t_ise_sentence sentence) {
	// TODO [LU]
}

void abort_on_sentence_error(t_ise_sentence sentence, t_ise_script* script) {
	notify_sentence_error(sentence);
	exit_with_error();
}

void execute_script() {
	t_ise_sentence sentence;
	while (!(sentence = next_sentence()).empty) {
		wait_to_execute();

		t_sentence_process_result result;
		if (sentence.operation.valido) {
			result = send_operation_to_coordinator(sentence.operation);
		} else {
			result = INVALID_SENTENCE;
		}
		notify_sentence_processed_to_planifier(result);

		if (result != OK) {
			abort_on_sentence_error(sentence, script);
		}
	}
}

void wait_to_execute() {
	// TODO [LU]
}

void notify_sentence_processed_to_planifier(t_sentence_process_result result) {
	// TODO [LU]
}

t_sentence_process_result send_operation_to_coordinator(
		t_esi_operacion sentence) {
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
