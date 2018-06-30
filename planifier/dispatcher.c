/*
 * dispatcher.c
 *
 *  Created on: 29 jun. 2018
 *      Author: utnso
 */

#include "dispatcher.h"

bool paused = false;

void pause_dispatcher() {
	paused = true;
}

void resume_dispatcher() {
	paused = false;
}

void manage_flow() {
	if (paused && RUNNING_ESI != NEXT_RUNNING_ESI) {
		pthread_cond_signal(&paused_planification);
		log_debug(logger, "Ready to pause dispatcher...");
		pthread_cond_wait(&resume_planification, &dispatcher_manager);
		log_debug(logger, "Ready to resume dispatching...");
	}
}

void dispatch() {
	while(true) {
		log_debug(logger, "Waiting to start planification");
		pthread_mutex_lock(&dispatcher_manager);
		manage_flow();

		log_debug(logger, "Started/resumed planification");
		esi* esi = esi_se_va_a_ejecutar();
		if (esi->instrucction_pointer == esi->cantidad_de_instrucciones) {
			log_debug(logger, "Reached end of ESI%ld", esi->id);
			finish_esi(RUNNING_ESI);
		}
		if (send_esi_to_run(RUNNING_ESI)) {
			log_debug(logger, "Told ESI%ld to run", RUNNING_ESI);
		} else {
			log_error(logger, "Could not tell ESI%ld to run", RUNNING_ESI);
			// TODO: probablemente habría sacarlo de todas las colas
			RUNNING_ESI = NEXT_RUNNING_ESI;
			pthread_mutex_unlock(&dispatcher_manager);
			continue;
		}
		log_debug(logger, "Waiting for execution result...");
		if (wait_execution_result(RUNNING_ESI)) {
			log_debug(logger, "Execution result received, incrementing cpu_time (current cpu_time: %ld)", get_current_time());
		} else {
			log_error(logger, "Execution result could not be received");
			// TODO: probablemente habría sacarlo de todas las colas
			RUNNING_ESI = NEXT_RUNNING_ESI;
			pthread_mutex_unlock(&dispatcher_manager);
			continue;
		}
		cpu_time_incrementate();
		log_debug(logger, "cpu_time incremented to %ld", get_current_time());
		borrado_de_finish();
		pthread_mutex_unlock(&dispatcher_manager);
	}
}

void init_dispatcher() {
	pthread_mutex_lock(&dispatcher_manager);
	pthread_t orchestrator;
	if (pthread_create(&orchestrator, NULL, (void*) dispatch, NULL) < 0) {
		log_error(logger, "Could not start dispatcher");
		exit_gracefully(EXIT_FAILURE);
	}
}
