/*
 * status_cmd.c
 *
 *  Created on: 7 jul. 2018
 *      Author: utnso
 */

#include "status_cmd.h"

char* buffer;

bool is_empty(char* string) {
	return string_is_empty(string);
}

void append(char* field, char* value) {
	if (!is_empty(value)) {
		string_append_with_format(&buffer, "\n%s: {%s}", field, value);
	}
}

command_result status_cmd(command command) {
	char* key = (char*) list_get(command.args, 0);
	buffer = string_new();

	append("value", get_key_param(value, key));
	char* instance_name = get_key_param(instance, key);
	append("instance", instance_name);
	append("calculated_instance", is_empty(instance_name)? get_key_param(calculated_instance, key) : "");
	append("waiting", get_all_waiting_for_resource_as_string(key, ", "));

	command_result result;
	result.code = COMMAND_OK;
	memcpy(result.content, buffer, strlen(buffer));
	free(buffer);
	return result;
}
