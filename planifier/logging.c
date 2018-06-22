/*
 * logging.c
 *
 *  Created on: 20 jun. 2018
 *      Author: utnso
 */

#include "logging.h"

t_log * logger;

void init_logger() {
	logger = log_create("planifier.log", "planifier", false, LOG_LEVEL_INFO);
//	logger = log_create("planifier.log", "planifier", false, LOG_LEVEL_DEBUG);
}

