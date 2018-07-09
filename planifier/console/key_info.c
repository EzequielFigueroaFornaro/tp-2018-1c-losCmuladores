/*
 * coordinator_connector.c
 *
 *  Created on: 8 jul. 2018
 *      Author: utnso
 */

#include "key_info.h"

int c_socket;
int c_port;
char* c_ip;

char* NO_VALUE = "No posee";

void open_new_coordinator_connection() {
	c_socket = connect_to(c_ip, c_port);
}

execution_result expected_result_for_param(key_param param) {
	switch (param) {
	case value: return KEY_FOUND;
	case instance: return KEY_FOUND;
	case calculated_instance: return INSTANCE_AVAILABLE_FOR_KEY;
	}
	log_error(logger, "No expected result found for key param %d", param);
	return 0;
}

message_type request_type_for_param(key_param param) {
	switch(param) {
	case value: return GET_KEY_VALUE;
	case instance: return GET_INSTANCE;
	case calculated_instance: return CALCULATE_INSTANCE;
	}
	log_error(logger, "No message_type found for key param %d", param);
	return 0;
}

char* empty(key_param param) {
	return param == value? NO_VALUE : "";
}

void send_request_finished() {
	if (send(c_socket, &KEY_INFO_REQUEST_FINISHED, sizeof(message_type), 0) < 0) {
		log_warning(console_log,
				"Could not inform coordinator of key info request finished");
	}
	close(c_socket);
}

char* get_key_param(key_param param, char* key) {
	open_new_coordinator_connection();

	int key_size = strlen(key) + 1;
	int message_size = sizeof(message_type)*2 + sizeof(module_type) + sizeof(int) + key_size;
	void* message = malloc(message_size);
	void* offset = message;
	concat_value(&offset, &KEY_INFO_REQUEST, sizeof(message_type));
	int self_module = PLANIFIER;
	concat_value(&offset, &self_module, sizeof(message_type));
	message_type request_type = request_type_for_param(param);
	concat_value(&offset, &request_type, sizeof(message_type));
	concat_string(&offset, key, key_size);

	if (send(c_socket, message, message_size, 0) < 0) {
		log_error(console_log, "Unable to send request to coordinator to get key param %d", param);
		close(c_socket);
		return empty(param);
	}
	int result;
	if (recv_int(c_socket, &result) <= 0) {
		log_error(console_log, "Error while receiving response code from coordinator for key param %d", param);
		close(c_socket);
		return empty(param);
	}


	if (result != expected_result_for_param(param)) {
		send_request_finished();
		return empty(param);
	}
	char* param_value;
	if (recv_string(c_socket, &param_value) <= 0) {
		log_error(console_log, "Error while receiving response from coordinator for key param %d", param);
		return empty(param);
	}
	send_request_finished();
	return param_value;
}

void set_coordinator_connection_params(char* ip, int port) {
	c_port = port;
	c_ip = ip;
}
