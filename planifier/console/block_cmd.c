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
		result.content = string_from_format("ESI with id '%ld' does not exist", esi_id);
		return result;
	}

	esi* esi = get_esi_by_id(esi_id);
	if (esi->estado != CORRIENDO && esi->estado != LISTO) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("ESI%ld isn't either running or ready!", esi_id);
		return result;
	}

	if (!string_is_blank(esi->blocking_resource)) {
		result.code = COMMAND_ERROR;
		result.content =
				string_from_format(
						"ESI%ld already has a resource: '%s'. ESIs can't have more than 1 resource taken",
						esi_id, esi->blocking_resource);
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

	result.content = string_from_format("ESI%s %s", esi_id,
			(took_resource ?
					string_from_format("was given resource '%s'", resource) :
					string_from_format("is now in the waiting queue of resource '%s'", resource)));
	return result;
}
