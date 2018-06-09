/*
 * block_command.c
 *
 *  Created on: 9 jun. 2018
 */

#include "block_cmd.h"

typedef struct {
	char* esi_id;
	char* resource;
} command_info;

long to_long(char* esi_id) {
	return atol(esi_id);
}

void add_to_blocked_queue(char* resource, char* esi_id) {
	pthread_mutex_lock(&map_boqueados);
	if (!dictionary_has_key(esis_bloqueados_por_recurso, resource)) {
		dictionary_put(esis_bloqueados_por_recurso, resource, queue_create());
	}
	t_queue* esis = dictionary_get(esis_bloqueados_por_recurso, resource);
	long id = to_long(esi_id);
	queue_push(esis, &id);
	pthread_mutex_unlock(&map_boqueados);
}

void try_block(command_info* command_info) {
	add_to_blocked_queue(command_info->resource, command_info->esi_id);
	block_esi(to_long(command_info->esi_id));
}

void async_block(command_info* command_info) {
	pthread_t thread;
	pthread_create(&thread, NULL, (void*) try_block, (void*) command_info);
	pthread_join(thread, NULL);
	free(command_info);
}

command_result block_cmd(command command) {
	command_result result;

	char* resource = list_get(command.args, 0);
	char* esi_id = list_get(command.args, 1);

	if (esi_map == NULL) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("ESI with id '%s' does not exist", esi_id);
		return result;
	}

	esi* esi = dictionary_get(esi_map, esi_id);
	if (esi == NULL) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("ESI with id '%s' does not exist", esi_id);
		return result;
	}

	pthread_t thread;
	command_info* command_info = malloc(sizeof(command_info));
	command_info->esi_id = esi_id;
	command_info->resource = resource;
	pthread_create(&thread, NULL, (void*) async_block, (void*) command_info);

	result.code = COMMAND_OK;
	result.content = string_from_format("Ok! ESI %s will be blocked in the next possible opportunity", esi_id);
	return result;
}
