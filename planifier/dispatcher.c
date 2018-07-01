/*
 * dispatcher.c
 *
 *  Created on: 29 jun. 2018
 *      Author: utnso
 */

#include "dispatcher.h"

bool paused = false;
bool permission_to_block_acquired = false;

void pause_dispatcher() {
	paused = true;
	if (pthread_mutex_trylock(&dispatcher_manager) == 0) {
		pthread_mutex_unlock(&dispatcher_manager);
		pthread_mutex_lock(&pause_manager);
		pthread_cond_wait(&paused_planification, &pause_manager);
	}
}

void resume_dispatcher() {
	paused = false;
	pthread_cond_signal(&resume_planification);
	pthread_mutex_unlock(&pause_manager);
}

void acquire_permission_to_block() {
	permission_to_block_acquired = true;
	pthread_mutex_lock(&permission_to_block_manager);
	pthread_cond_wait(&permission_to_block, &permission_to_block_manager);
}

void release_permission_to_block() {
	permission_to_block_acquired = false;
	pthread_cond_signal(&permission_to_block_released);
	pthread_mutex_unlock(&permission_to_block_manager);
}

void on_atomic_execution() {
	if (paused) {
		pthread_cond_signal(&paused_planification);
		log_debug(logger, "Pausing dispatcher...");
		pthread_cond_wait(&resume_planification, &dispatcher_manager);
		log_debug(logger, "Resuming dispatching...");
	}

	if (permission_to_block_acquired) {
		pthread_cond_signal(&permission_to_block);
		log_debug(logger, "Sent permission to console to block ESI");
		pthread_cond_wait(&permission_to_block_released, &dispatcher_manager);
		log_debug(logger, "An ESI was blocked by console");
	}
}

esi* get_esi(long esi_id) {
	esi* esi;
	pthread_mutex_lock(&esi_map_mtx);
	esi = dictionary_get(esi_map, id_to_string(RUNNING_ESI));
	pthread_mutex_unlock(&esi_map_mtx);
	return esi;
}

void dispatch() {
	while(true) {
		log_debug(logger, "Waiting to dispatch...");
		pthread_mutex_lock(&dispatcher_manager);

		long current_esi = esi_se_va_a_ejecutar();
		if (current_esi == 0) {
			log_debug(logger, "Ready list is empty, sleeping...");
			// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
			continue;
		}

		esi* esi = get_esi(current_esi);
		if (esi->instruction_pointer == esi->cantidad_de_instrucciones) {
			log_debug(logger, "ESI%ld has finished!", esi->id);
			finish_esi(esi->id);
			pthread_mutex_unlock(&dispatcher_manager);
			continue;
		}

		on_atomic_execution();
		if (send_esi_to_run(current_esi)) {
			log_debug(logger, "Told ESI%ld to run", current_esi);
		} else {
			log_error(logger, "Could not tell ESI%ld to run", current_esi);
			finish_esi(current_esi);
			pthread_mutex_unlock(&dispatcher_manager);
			continue;
		}
		log_debug(logger, "Waiting for execution result...");
		if (!wait_execution_result(current_esi)) {
			log_error(logger, "Execution result could not be received");
			finish_esi(current_esi);
			pthread_mutex_unlock(&dispatcher_manager);
			continue;
		}
		log_debug(logger, "Execution result received, incrementing cpu_time (current cpu_time: %ld)", get_current_time());
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
