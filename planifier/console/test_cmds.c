/*
 * test_add_cmd.c
 *
 *  Created on: 21 jun. 2018
 *      Author: utnso
 */

#include "test_cmds.h"

command_result add_cmd(command command) {
	pthread_mutex_lock(&esi_map_mtx);

	char* esi_id = list_get(command.args, 0);
	esi* new_esi = malloc(sizeof(esi));
	new_esi->id = atol(esi_id);

	dictionary_put(esi_map, esi_id, new_esi);

	pthread_mutex_unlock(&esi_map_mtx);
	command_result result = base_command_result(COMMAND_OK);
	result.content = string_from_format("ESI%s agregado", esi_id);
	return result;
}

command_result list_esis_cmd(command command) {
	pthread_mutex_lock(&esi_map_mtx);

	char* esis = string_new();
	void to_string(char* key, esi* esi) {
		string_append_with_format(&esis, "ESI id string: %s, ESI id long: %ld", key, esi->id);
	}

	dictionary_iterator(esi_map, (void*) to_string);
	pthread_mutex_unlock(&esi_map_mtx);
	command_result result = base_command_result(COMMAND_OK);
	result.content = esis;
	return result;
}
