/*
 * kill_cmd.c
 *
 *  Created on: 7 jul. 2018
 *      Author: utnso
 */

#include "kill_cmd.h"

command_result kill_cmd(command command) {
	command_result result;

	char* esi_id = list_get(command.args, 0);

	esi* esi = get_esi_by_id(id_as_long(esi_id));
	if (esi == NULL) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("El ESI%s no existe!", esi_id);
		return result;
	}

	if (esi->estado != BLOQUEADO) {
		result.code = COMMAND_ERROR;
		result.content = string_from_format("El ESI%s no esta bloqueado", esi_id);
		return result;
	}

	finish_esi(id_as_long(esi_id));

	result.code = COMMAND_OK;
	result.content = string_from_format(
			"El ESI fue finalizado (estado del ESI: %s)", esi_id,
			esi_to_string(esi));
	return result;
}
