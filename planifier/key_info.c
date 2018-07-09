/*
 * coordinator_connector.c
 *
 *  Created on: 8 jul. 2018
 *      Author: utnso
 */

#include "key_info.h"

int c_socket;

char* NO_VALUE = "No posee";

execution_result expected_result_for_param(key_param param) {
	switch (param) {
	case value: return KEY_FOUND;
	case instance: return KEY_FOUND;
	case calculated_instance: return CALCULATE_INSTANCE_OK;
	}
	log_error(logger, "No expected result found for key param %d", param);
	return 0;
}

message_type message_type_for_param(key_param param) {
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

char* get_key_param(key_param param, char* key) {
	int key_size = strlen(key) + 1;
	int message_size = sizeof(message_type) + sizeof(int) + key_size;
	void* message = malloc(message_size);
	void* offset = message;
	concat_value(&offset, &param, sizeof(message_type));
	concat_string(&offset, key, key_size);

	if (send(c_socket, message, message_size, 0) < 0) {
		return empty(param);
	}
	int result;
	if (recv_int(c_socket, &result) <= 0 || result != expected_result_for_param(param)) {
		return empty(param);
	}
	char* param_value;
	if (recv_string(c_socket, &param_value) <= 0) {
		return empty(param);
	}

	return param_value;
}

void set_coordinator_socket(int socket) {
	c_socket = socket;
}
