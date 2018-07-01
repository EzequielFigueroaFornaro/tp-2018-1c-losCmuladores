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
	log_on_both("Planification paused");

	printf("Press ENTER to resume planification... ");
	getchar();

	resume_dispatcher();
	log_info(logger, "Planification resumed");

	command_result result = base_command_result(COMMAND_OK);
	result.content = "Planification resumed!";
	return result;
}
