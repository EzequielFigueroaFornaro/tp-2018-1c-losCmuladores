/*
 ============================================================================
 Name        : ise.c
 Author      : losCmuladores
 ============================================================================
 */

#include "ise.h"

void assert_not_blank(char* msg, char* arg);

int main(int argc, char* argv[]) {
	init_logger();

	assert_not_blank("Configuration file required!", argv[1]);
	assert_not_blank("Script file required!", argv[2]);

	load_config(argv[1]);
	load_script(argv[2]);

	start_signal_listener();

	connect_to_planifier();
	connect_to_coordinator();

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
	if (recv_long(get_socket(PLANIFIER), &self_id) <= 0) {
		log_error(logger, "Could not receive my id from planifier. Aborting");
		exit_with_error();
	}
	log_info(logger, "Connected to planifier! My id is: %ld", self_id);
}

void connect_to_coordinator() {
	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(long);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &MODULE_CONNECTED, sizeof(message_type));
	concat_value(&offset, &self_module_type, sizeof(module_type));
	concat_value(&offset, &self_id, sizeof(long));

	handshake(COORDINATOR, buffer, message_size);
	log_info(logger, "Connected to coordinator! Now waiting planifier signal to execute...");
}

void execute_script() {
	t_ise_sentence current_sentence;
	while(!(current_sentence = get_sentence_to_execute()).empty) {
		wait_to_execute();

		execution_result result;
		if (current_sentence.operation.valido) {
			result = send_sentence_to_coordinator(current_sentence.operation);
		} else {
			result = PARSE_ERROR;
		}
		handle_execution_result(result);
	}
}

void handle_execution_result(execution_result result) {
	log_info(logger, "Received result from coordinator: %s", get_execution_result_description(result));
	notify_planifier(result);
	if (result == KEY_BLOCKED) {
		log_info(logger, "Going to retry last sentence next...");
		set_retry_current_sentence(true);
	} else {
		set_retry_current_sentence(false);
	}
}

void wait_to_execute() {
	message_type msg;
	int socket = get_socket(PLANIFIER);
	while ((msg = recv_message(socket)) != ISE_EXECUTE) {
		if (msg <= 0) {
			log_error(logger, "Lost connection with planifier! Aborting");
			exit_with_error(); // TODO [Lu] Debería hacer esto?
		}
		if (msg == ISE_STOP) {
			log_info(logger, "Received stop signal from planifier. Waiting to continue...");
		} else if (msg == ISE_KILL) {
			log_info(logger, "Received abort signal from planifier. Aborting");
			exit_with_error();
		} else {
			log_error(logger, "Received unknown signal from planifier. Will keep on waiting just in case...");
		}
	}
	log_info(logger, "Received execute signal from planifier");
}

void notify_planifier(execution_result result) {
	log_info(logger, "Notifying sentence execution to planifier");
	int message_size = sizeof(message_type) + sizeof(execution_result);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &EXECUTION_RESULT, sizeof(message_type));
	concat_value(&offset, &result, sizeof(execution_result));
	if (send(get_socket(PLANIFIER), buffer, message_size, 0) < 0) {
		log_error(logger, "Could not notify planifier of sentence execution");
	}
	free(buffer);
}

execution_result send_sentence_to_coordinator(t_esi_operacion operation) {
	t_sentence* sentence = map_to_sentence(operation);
	t_buffer sentence_buffer = serialize_sentence(sentence);

	log_info(logger, "Sending sentence to coordinator: %s",	sentence_to_string(sentence));

	int socket = get_socket(COORDINATOR);
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
	if (recv_int(socket, &result) <= 0) {
		log_error(logger, "Error while trying to receive sentence execution result");
		exit_with_error();
	}
	return result;
}

void assert_not_blank(char* msg, char* arg) {
	if (arg == NULL || string_is_empty(arg)) {
		log_error(logger, msg);
		exit_with_error();
	}
}

