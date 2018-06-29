/*
 * pause_cmd.c
 *
 *  Created on: 24 jun. 2018
 *      Author: utnso
 */

#include "pause_cmd.h"

command_result pause_cmd(command command) {
	pause_dispatcher();
	pthread_mutex_lock(&pause_cmd_manager);
	pthread_cond_wait(&paused_planification, &pause_cmd_manager);
	log_info(logger, "Planification paused");
	printf("Press ENTER to resume planification... ");
	getchar();
	resume_dispatcher();
	pthread_cond_signal(&resume_planification);
	pthread_mutex_unlock(&pause_cmd_manager);
	log_info(logger, "Planification resumed");

	command_result result = base_command_result(COMMAND_OK);
	result.content = "Planification resumed!";
	return result;
}
