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
		string_append_with_format(&buffer, "\t%s: %s\n", field, value);
	}
}

char* get_waiting(char* key) {
	char* waiting = get_all_waiting_for_resource_as_string(key, ", ");
	return is_empty(waiting)? "No hay" : string_from_format("{ %s }", waiting);
}

command_result status_cmd(command command) {
	command_result result;

	char* key = (char*) list_get(command.args, 0);
	buffer = string_new();

	t_queue* esis = get_all_waiting_for_resource(key);
	if ((esis == NULL || queue_is_empty(esis)) && !resource_taken(key)) {
		result.code = COMMAND_ERROR;
		result.content = "La clave no esta activa/no existe";
		return result;
	}

	char* instance_name = get_key_param(instance, key);

	string_append(&buffer, "\n");
	append("valor", get_key_param(value, key));
	append("instancia", instance_name);
	append("instancia_calculada", is_empty(instance_name)? get_key_param(calculated_instance, key) : "");
	append("esperando", get_waiting(key));

	result.code = COMMAND_OK;
	result.content = buffer;
	return result;
}
