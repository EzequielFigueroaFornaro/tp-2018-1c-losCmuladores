/*
 * pause_cmd.c
 *
 *  Created on: 24 jun. 2018
 *      Author: utnso
 */

#include "pause_cmd.h"

command_result pause_cmd(command command) {
	pthread_mutex_lock(&start_planification);
	log_info(logger, "Planification paused");
	system("pause");
	pthread_mutex_unlock(&start_planification);
	log_info(logger, "Planification resumed");
	return base_command_result(COMMAND_OK);
}
