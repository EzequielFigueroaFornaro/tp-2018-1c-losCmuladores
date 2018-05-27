/*
 ============================================================================
 Name        : ise.c
 Author      : losCmuladores
 ============================================================================
 */

#include "ise.h"

void load_script(char* file_name);
void exit_with_error();
void handshake(module_type module, void* handshake_message, int handshake_message_size);
t_sentence* map_to_sentence(t_esi_operacion operation);

int main(int argc, char* argv[]) {
	init_logger();
	load_config(argv[1], COORDINATOR, PLANIFIER);
//	load_script(argv[2]);

//	connect_to_planifier();
	connect_to_coordinator();

//	execute_script();
	while(true);
	exit_gracefully(EXIT_SUCCESS);
}

void connect_to_planifier() {
	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(long);
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &MODULE_CONNECTED, sizeof(message_type));
	concat_value(&offset, &ISE, sizeof(module_type));
	long script_size = queue_size(script->lines);
	concat_value(&offset, &script_size, sizeof(script_size));

	handshake(PLANIFIER, buffer, message_size);
	recv_string(get_socket(PLANIFIER), &my_id); // TODO [Lu] validar
}

void connect_to_coordinator() {
	int id_len = strlen(my_id) + 1;
	int message_size = sizeof(message_type) + sizeof(module_type) + sizeof(int) + id_len;
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &MODULE_CONNECTED, sizeof(message_type));
	concat_value(&offset, &ISE, sizeof(module_type));
	concat_string(&offset, my_id, id_len);

	handshake(COORDINATOR, buffer, message_size);
	log_info(logger, "Sending %d bytes", message_size);
}

void handshake(module_type module, void* handshake_message, int handshake_message_size) {
	int socket = get_socket(module);
	socket = connect_to(get_ip(module), get_port(module));
		if (socket < 0) {
			log_error(logger, /* TODO [Lu] loggear errno*/"Could not connect to coordinator");
			exit_with_error();
		}

	int send_result = send(socket, handshake_message, handshake_message_size, 0);
	free(handshake_message);
	if (send_result < 0) {
		log_error(logger, "Could not send handshake");
		exit_with_error();
	}

	if (recv_message(socket) != CONNECTION_SUCCESS) {
		log_error(logger, "Rejected handshake");
		exit_with_error();
	}
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
			result = send_sentence_to_coordinator(sentence.operation);
		} else {
			result = INVALID_SENTENCE;
		}

		if (result != OK) {
			notify_error(); // TODO [Lu] Revisar si esto va
			exit_with_error();
		}
		notify_planifier();
	}
}

void wait_to_execute() {
	while(recv_message(planifier_socket) != ISE_EXECUTE);
}

void notify_planifier() {
	message_type message_type = SENTENCE_EXECUTED_OK;
	if (send(planifier_socket, &message_type, sizeof(message_type), 0) < 0) {
		log_error(logger, "Could not notify planifier of sentence execution");
	}
}

t_sentence_process_result send_sentence_to_coordinator(t_esi_operacion operation) {
	t_sentence* sentence = map_to_sentence(operation);
	destruir_operacion(operation);
	t_buffer sentence_buffer = serialize_sentence(sentence);

	int socket = get_socket(COORDINATOR);
	int send_result = send(socket, sentence_buffer.buffer_content, sentence_buffer.size, 0);
	free(sentence_buffer.buffer_content);
	if (send_result < 0) {
		log_error(logger, "Could not send operation.");
		exit_with_error();
	}
	if (recv_message(socket) != SENTENCE_RESULT) {
		log_error(logger, "Unexpected message received when waiting for sentence execution result");
		exit_with_error();
	}
	t_sentence_process_result result;
	int message_type_result = recv(socket, &result, sizeof(t_sentence_process_result), MSG_WAITALL);
	if (message_type_result <= 0) {
		log_error(logger, "Error while trying to receive sentence execution result");
		exit_with_error();
	}
	return result;
}

t_sentence* map_to_sentence(t_esi_operacion operation) {
	t_sentence* sentence = malloc(sizeof(t_sentence));
	switch(operation.keyword) {
	case GET:
		sentence->operation_id = GET_SENTENCE;
		sentence->key = operation.argumentos.GET.clave;
		break;
	case SET:
		sentence->operation_id = SET_SENTENCE;
		sentence->key = operation.argumentos.SET.clave;
		sentence->value = operation.argumentos.SET.valor;
		break;
	case STORE:
		sentence->operation_id = STORE_SENTENCE;
		sentence->key = operation.argumentos.STORE.clave;
		break;
	default:
		log_error(logger,"Unexpected error: sentence keyword not recognized");
		exit_with_error();
	}
	return sentence;
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
	destroy_config();
	exit(code);
}

void exit_with_error() {
	exit_gracefully(EXIT_FAILURE);
}
