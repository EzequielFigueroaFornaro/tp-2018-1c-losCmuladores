/*
 ============================================================================
 Name        : ise.c
 Author      : losCmuladores
 ============================================================================
 */

#include "ise.h"

int main(int argc, char* argv[]) {
	init_logger();
	load_config(argv[1]);
	load_script(argv[2]);

	connect_to_coordinator();
	connect_to_planifier();

	execute_script();
	exit_gracefully(EXIT_SUCCESS);
}

void connect_to_planifier() {
	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(long);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &MODULE_CONNECTED, sizeof(message_type));
	concat_value(&offset, &self_module_type, sizeof(module_type));
	long script_size = get_script_size();
	concat_value(&offset, &script_size, sizeof(script_size));

	handshake(PLANIFIER, buffer, message_size);
	recv_value(get_socket(PLANIFIER), &my_id); // TODO [Lu] validar
}

void connect_to_coordinator() {
	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(int);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &MODULE_CONNECTED, sizeof(message_type));
	concat_value(&offset, &self_module_type, sizeof(module_type));
	concat_value(&offset, &my_id, sizeof(int));

	handshake(COORDINATOR, buffer, message_size);
}

void execute_script() {
	t_ise_sentence sentence;

	while (!(sentence = next_sentence()).empty) {
		wait_to_execute();

		int result;
		if (sentence.operation.valido) {
			result = send_sentence_to_coordinator(sentence.operation);
		} else {
			result = PARSE_ERROR;
		}
		handle_execution_result(result);
	}
}

void handle_execution_result(execution_result result) {
	notify_planifier(result);
	if (result == KEY_BLOCKED) {
		// guardo la instruccion
	} else if (result != OK) {
		log_error(logger, "%s. Aborting", get_execution_result_description(result));
		exit_with_error();
	}
}

void wait_to_execute() {
	log_info(logger, "Waiting to execute...");
	while(recv_message(get_socket(PLANIFIER)) != ISE_EXECUTE);
}

void notify_planifier(execution_result result) {
	int message_size = sizeof(message_type) + sizeof(result);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &EXECUTION_RESULT, sizeof(message_type));
	concat_value(&offset, &result, sizeof(result));
	if (send(get_socket(PLANIFIER), buffer, sizeof(message_type), 0) < 0) {
		log_error(logger, "Could not notify planifier of sentence execution");
	}
	free(buffer);
	free(offset);
}

execution_result send_sentence_to_coordinator(t_esi_operacion operation) {
	t_sentence* sentence = map_to_sentence(operation);
	destruir_operacion(operation);
	t_buffer sentence_buffer = serialize_sentence(sentence);

	int socket = get_socket(0);
	int send_result = send(socket, sentence_buffer.buffer_content, sentence_buffer.size, 0);
	free(sentence_buffer.buffer_content);
	if (send_result < 0) {
		log_error(logger, "Could not send operation.");
		exit_with_error();
	}
	if (recv_message(socket) != EXECUTION_RESULT) {
		log_error(logger, "Unexpected message received when waiting for sentence execution result");
		exit_with_error();
	}

	int result;
	int message_type_result = recv(socket, &result, sizeof(int), MSG_WAITALL);
	if (message_type_result <= 0) {
		log_error(logger, "Error while trying to receive sentence execution result");
		exit_with_error();
	}
	return result;
}

