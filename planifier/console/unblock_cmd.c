/*
 * unblock_cmd.c
 *
 *  Created on: 30 jun. 2018
 *      Author: utnso
 */

#include "unblock_cmd.h"

void unblock_resource(char* resource) {
	dictionary_remove(recurso_tomado_por_esi, resource);
}

command_result unblock_cmd(command command) {
	char* resource = list_get(command.args, 0);

	pthread_mutex_lock(&blocked_by_resource_map_mtx);
	t_queue* esis = get_all_waiting_for_resource(resource);

	command_result result;
	if (esis == NULL || queue_is_empty(esis)) {
		pthread_mutex_lock(&blocked_resources_map_mtx);
		if (resource_taken(resource)) {
			unblock_resource(resource);
			result.code = COMMAND_OK;
			result.content = string_from_format("No ESIs were waiting. Key %s was unlocked", resource);
		} else {
			result.code = COMMAND_ERROR;
			result.content = string_from_format("Resource %s not found");
		}
		pthread_mutex_unlock(&blocked_resources_map_mtx);
	} else {
		long* esi_id = queue_pop(esis);
		unblock_esi(*esi_id);
		result.code = COMMAND_OK;
		result.content = string_from_format("ESI%ld was unblocked from waiting for resource %s", *esi_id, resource);
	}

	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
	return result;
}

