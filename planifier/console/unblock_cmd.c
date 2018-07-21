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
		if (resource_taken_by_any_esi(resource)) {
			unblock_resource(resource);
			result.code = COMMAND_OK;
			result.content = string_from_format(
					"No hay ESIs esperando. El recurso %s fue desbloqueado",
					resource);
		} else {
			result.code = COMMAND_ERROR;
			result.content = string_from_format("El recurso %s no esta tomado por nadie y nadie esta esperando", resource);
		}
		pthread_mutex_unlock(&blocked_resources_map_mtx);
	} else {
		pthread_mutex_lock(&esi_map_mtx_6);

		long* esi_id = queue_pop(esis);
		while ((!is_valid_esi(*esi_id)) && esi_id!=null) {
			esi_id = queue_pop(esis);
		}
		if(esi_id!=NULL) {
			unblock_esi(*esi_id);
			result.code = COMMAND_OK;
			result.content = string_from_format(
					"El ESI%ld fue desbloqueado y ya no espera por el recurso %s",
					*esi_id, resource);
		} else {
			pthread_mutex_lock(&blocked_resources_map_mtx);
			if (resource_taken_by_any_esi(resource)) {
				unblock_resource(resource);
				result.code = COMMAND_OK;
				result.content = string_from_format(
						"No hay ESIs esperando. El recurso %s fue desbloqueado",
						resource);
			} else {
				result.code = COMMAND_ERROR;
				result.content = string_from_format("El recurso %s no esta tomado por nadie y nadie esta esperando", resource);
			}
			pthread_mutex_unlock(&blocked_resources_map_mtx);
		}


		pthread_mutex_unlock(&esi_map_mtx_6);

	}

	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
	return result;
}

