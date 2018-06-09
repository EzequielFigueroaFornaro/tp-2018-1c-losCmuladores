/*
 * list_cmd.c
 *
 *  Created on: 8 jun. 2018
 */
#include "list_cmd.h"

char* buffer;

void add_to_buffer(long* esi_id) {
	string_append_with_format(&buffer, "ESI%ld\n", *esi_id);
}

char* to_string(t_queue* queue) {
	if (queue == NULL) {
		return "";
	}
	buffer = string_new();
	list_iterate(queue->elements, (void *)add_to_buffer);
	return buffer;
}

command_result list_cmd(command command) {
	char* resource = (char*) list_get(command.args, 0);
	t_queue* blocked_esis = (t_queue*) dictionary_get(esis_bloqueados_por_recurso, resource);
	command_result result = base_command_result(COMMAND_OK);
	result.content = to_string(blocked_esis);
	return result;
}
