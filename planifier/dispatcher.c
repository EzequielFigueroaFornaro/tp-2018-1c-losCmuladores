/*
 * dispatcher.c
 *
 *  Created on: 29 jun. 2018
 *      Author: utnso
 */

#include "dispatcher.h"

bool paused = false;
bool permission_to_block_acquired = false;

void acquire_dispatcher() {
	pthread_mutex_lock(&dispatcher_manager);
}

void acquire_next_and_running_semaphores() {
	pthread_mutex_lock(&running_esi_mtx_1);
	pthread_mutex_lock(&next_running_esi_mtx_2);
}

void unlock_next_and_running_semaphores() {
	pthread_mutex_unlock(&next_running_esi_mtx_2);
	pthread_mutex_unlock(&running_esi_mtx_1);
}

void atomic_execution() {
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
//	acquire_next_and_running_semaphores();
}

bool valid_esi_status(esi* esi) {
	if (esi->estado == BLOQUEADO) {
		if (NEXT_RUNNING_ESI == 0) {
			log_debug(logger, "ESI%ld was running but was evicted and ready list is empty, sleeping...", esi->id);
//			unlock_next_and_running_semaphores();
			// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
		} else {
			log_debug(logger, "ESI%ld was running but was evicted, continuing with next ESI...", esi->id);
//			unlock_next_and_running_semaphores();
			pthread_mutex_unlock(&dispatcher_manager);
		}
		return false;
	}
	return true;
}

void check_execution_error(execution_result result) {
	if (result == KEY_UNREACHABLE
			|| result == KEY_LOCK_NOT_ACQUIRED
			|| result == KEY_TOO_LONG
			|| result == KEY_NOT_FOUND) {
		log_info(logger, "Finishing ESI%ld because of execution error",
				RUNNING_ESI);
//		unlock_next_and_running_semaphores();
		finish_esi(RUNNING_ESI);
//		acquire_next_and_running_semaphores();
	}
}

void dispatch() {
	while(true) {
		log_debug(logger, "Waiting to dispatch...");
		acquire_dispatcher();

		long current_esi = esi_se_va_a_ejecutar();
		if (current_esi == 0) {
			log_debug(logger, "Ready list is empty, sleeping...");
			// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
//			unlock_next_and_running_semaphores();
			continue;
		}

		esi* esi = get_esi_by_id(current_esi);
		if (esi->instruction_pointer == esi->cantidad_de_instrucciones) {
//			unlock_next_and_running_semaphores();
			pthread_mutex_unlock(&dispatcher_manager);

			log_debug(logger, "ESI%ld has finished!", esi->id);
			finish_esi(esi->id);
			continue;
		}

		atomic_execution();
		if (!valid_esi_status(esi)) {
			continue;
		}
		if (send_esi_to_run(RUNNING_ESI)) {
			log_debug(logger, "Told ESI%ld to run", RUNNING_ESI);
		} else {
//			unlock_next_and_running_semaphores();
			pthread_mutex_unlock(&dispatcher_manager);

			log_error(logger, "Could not tell ESI%ld to run", RUNNING_ESI);
			finish_esi(RUNNING_ESI);
			continue;
		}

		log_debug(logger, "Waiting for execution result...");
		int execution_result;
		if (!wait_execution_result(RUNNING_ESI, &execution_result)) {
//			unlock_next_and_running_semaphores();
			pthread_mutex_unlock(&dispatcher_manager);

			log_error(logger, "Execution result could not be received");
			finish_esi(RUNNING_ESI);
			continue;
		}

		log_info(logger, "Execution for ESI is: %s", get_execution_result_description(execution_result));
		check_execution_error(execution_result);

		log_debug(logger, "Incrementing cpu_time (current cpu_time: %ld)", get_current_time());
		cpu_time_incrementate();
		log_debug(logger, "cpu_time incremented to %ld", get_current_time());
		borrado_de_finish();

//		unlock_next_and_running_semaphores();
		pthread_mutex_unlock(&dispatcher_manager);
	}
}

void init_dispatcher() {
	acquire_dispatcher();
	pthread_t orchestrator;
	if (pthread_create(&orchestrator, NULL, (void*) dispatch, NULL) < 0) {
		log_error(logger, "Could not start dispatcher");
		exit_gracefully(EXIT_FAILURE);
	}
}


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
