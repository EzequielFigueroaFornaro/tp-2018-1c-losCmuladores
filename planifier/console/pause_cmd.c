/*
 * pause_cmd.c
 *
 *  Created on: 24 jun. 2018
 *      Author: utnso
 */

#include "pause_cmd.h"

command_result pause_cmd(command command) {
	log_info(console_log, "Trying to pause planification");

	pause_dispatcher();
	log_info(logger, "Planification paused");

	command_result result;
	result.code = COMMAND_OK;
	result.content = "Planificacion pausada!";
	return result;
}

command_result resume_cmd(command command) {
	log_info(console_log, "Trying to resume planification");
	resume_dispatcher();
	log_info(logger, "Planification resumed");

	command_result result;
	result.code = COMMAND_OK;
	result.content = "Planificacion reanudada!";
	return result;
}
