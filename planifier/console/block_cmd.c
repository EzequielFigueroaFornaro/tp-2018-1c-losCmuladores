/*
 * block_command.c
 *
 *  Created on: 9 jun. 2018
 */

#include "block_cmd.h"

command_result do_validations(char* resource, long esi_id) {
	command_result result;

	if (!esi_exists(esi_id)) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("El ESI%ld no existe", esi_id);
		return result;
	}

	esi* esi = get_esi_by_id(esi_id);
	if (esi->estado != CORRIENDO && esi->estado != LISTO && esi->estado != DESBLOQUEADO) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("El ESI%ld no esta ni ejecutando ni en listos!", esi_id);
		return result;
	}

	char* resource_taken = get_resource_taken_by_esi(esi_id);
	if (!string_is_empty(resource_taken)) {
		result.code = COMMAND_ERROR;
		result.content =
				string_from_format(
						"El ESI%ld ya tiene tomado un recurso: '%s'",
						esi_id, resource_taken);
		return result;
	}

	result.code = COMMAND_OK;
	return result;
}

command_result block_cmd(command command) {
	command_result result;

	char* resource = list_get(command.args, 0);
	char* esi_id = list_get(command.args, 1);
	long id = id_as_long(esi_id);

	result = do_validations(resource, id);
	if (result.code == COMMAND_ERROR) {
		return result;
	}

	bool took_resource;
	if (get_esi_by_id(id)->estado == CORRIENDO) {
		acquire_permission_to_block();
		took_resource = bloquear_recurso(resource, id);
		release_permission_to_block();
	} else {
		took_resource = bloquear_recurso(resource, id);
	}

	result.content = took_resource ?
					string_from_format("Se asigno el recurso '%s' al ESI%s", resource, esi_id) :
					string_from_format("El ESI%s ahora esta esperando por el recurso '%s'", resource);
	return result;
}
