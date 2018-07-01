/*
 * block_command.c
 *
 *  Created on: 9 jun. 2018
 */

#include "block_cmd.h"

command_result block_cmd(command command) {
	command_result result;

	char* resource = list_get(command.args, 0);
	char* esi_id = list_get(command.args, 1);

	if (!is_valid_esi(id_as_long(esi_id))) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("ESI with id '%s' does not exist", esi_id);
		return result;
	}

	esi* esi = dictionary_get(esi_map, esi_id);
	if (esi->estado != CORRIENDO || esi->estado != DESBLOQUEADO) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("ESI with id '%s' is finished and is waiting to be deleted", esi_id);
		return result;
	}

	acquire_permission_to_block();

	block_esi_by_resource(id_as_long(esi_id), resource);

	release_permission_to_block();

	result.code = COMMAND_OK;
	result.content = string_from_format("Ok! ESI %s will be blocked in the next possible opportunity", esi_id);
	return result;
}
