/*
 * dispatcher.c
 *
 *  Created on: 29 jun. 2018
 *      Author: utnso
 */

#include "dispatcher.h"

bool paused = false;
bool permission_to_block_acquired = false;

void atomic_execution() {
	if (paused) {
		pthread_cond_signal(&paused_planification);
		log_info(logger, "Pausando dispatcher...");
		pthread_cond_wait(&resume_planification, &dispatcher_manager);
		log_info(logger, "Reanudando dispatcher...");
	}

	if (permission_to_block_acquired) {
		pthread_cond_signal(&permission_to_block);
		log_info(logger, "Se habilito bloqueo desde consola...");
		pthread_cond_wait(&permission_to_block_released, &dispatcher_manager);
		log_info(logger, "Un ESI fue bloqueado desde consola");
	}
//	acquire_next_and_running_semaphores();
}

bool valid_esi_status(esi* esi) {
	if (esi->estado == BLOQUEADO) {
		if (NEXT_RUNNING_ESI == 0) {
			log_info(logger, "El ESI%ld fue desalojado y la cola de listos esta vacia, durmiendo...", esi->id);
//			unlock_next_and_running_semaphores();
			// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
		} else {
			log_info(logger, "El ESI%ld fue desalojado, se sigue con el siguiente ESI...", esi->id);
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
		log_info(logger, "Se finaliza el ESI%ld por error de ejecucion de sentencia",
				RUNNING_ESI);
//		unlock_next_and_running_semaphores();
		finish_esi(RUNNING_ESI);
//		acquire_next_and_running_semaphores();
	}
}

void dispatch() {
	while(true) {
		log_info(logger, "Esperando para despachar...");
		acquire_dispatcher();

		atomic_execution();

		if (RUNNING_ESI == 0) {
			replan_by_algorithm();
		}

		RUNNING_ESI = NEXT_RUNNING_ESI;
		modificar_estado(RUNNING_ESI, CORRIENDO);
		esi* esi = get_esi_by_id(RUNNING_ESI);
		esi->instruction_pointer++;

		if (send_esi_to_run(esi->id)) {
			log_info_highlight(logger,
					"Se dio aviso a %s (id: %ld) para que ejecute", esi->nombre,
					esi->id);
		} else {
			//			unlock_next_and_running_semaphores();
			pthread_mutex_unlock(&dispatcher_manager);

			log_error(logger, "Error al avisar a %s (id: %ld) que corra",
					esi->nombre, esi->id);
			finish_esi(RUNNING_ESI);
			continue;
		}

		log_info(logger, "Esperando resultado de ejecucion de %s (id: %ld)...",
				esi->nombre, esi->id);
		int result;
		if (!wait_execution_result(RUNNING_ESI, &result)) {
			//			unlock_next_and_running_semaphores();
			pthread_mutex_unlock(&dispatcher_manager);

			log_error(logger,
					"No se pudo recibir el resultado de ejecucion de %s (id: %ld)",
					esi->nombre, esi->id);
			finish_esi(RUNNING_ESI);
			continue;
		}

		log_info_highlight(logger, "Resultado de ejecucion de %s (id: %ld): %s",
				esi->nombre, esi->id, get_execution_result_description(result));
		check_execution_error(result);

		log_info(logger, "Incrementando clock de CPU (actual: %ld)", get_current_time());
		cpu_time_incrementate();
		log_debug(logger, "Clock de CPU incrementado (ahora es: %ld)",get_current_time());
		borrado_de_finish();


		if (esi->estado == BLOQUEADO) {
			esi->instruction_pointer--;
		}

		if(RUNNING_ESI != NEXT_RUNNING_ESI && NEXT_RUNNING_ESI != 0 && esi->estado == BLOQUEADO) {
			log_info(logger, "El ESI%ld fue desalojado, se continua con el proximo ESI", esi->id);
			pthread_mutex_unlock(&dispatcher_manager);
			continue;
		}

		// RUNNING_ESI == NEXT_RUNNING_ESI {

		if(NEXT_RUNNING_ESI == 0) {
			if (esi->estado == BLOQUEADO) {
				log_info(logger, "El ESI%ld fue desalojado y la cola de listos esta vacia, durmiendo...", esi->id);
			}
			RUNNING_ESI = 0;
			// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
			continue;
		}

		if (esi->instruction_pointer == esi->cantidad_de_instrucciones) {
			log_info_important(logger, "%s (id: %ld) finalizo su ejecucion!",
					esi->nombre, esi->id);
			finish_esi(esi->id);
			if (NEXT_RUNNING_ESI == 0) {
				RUNNING_ESI = 0;
				// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
				log_info(logger, "Finalizo el ultimo ESI (id: %ld) y no hay mas ESIs en la cola de listos...", esi->id);
				continue;
			}
		} else {
			log_info_important(logger,
					"El ESI%ld aun no termino, van %d/%d instrucciones",
					esi->id, esi->instruction_pointer, esi->cantidad_de_instrucciones);
		}
		pthread_mutex_unlock(&dispatcher_manager);
	}
}


//while(true) {
//		log_info(logger, "Esperando para despachar...");
//		acquire_dispatcher();
//
//		long current_esi = esi_se_va_a_ejecutar();
//		if (current_esi == 0) {
//			log_info(logger, "Cola de listos vacia, durmiendo...");
//			// No deslockeo dispatcher_manager para que se lockee en la proxima iteracion
////			unlock_next_and_running_semaphores();
//			continue;
//		}
//
//		esi* esi = get_esi_by_id(current_esi);
//		if (esi->instruction_pointer == esi->cantidad_de_instrucciones) {
////			unlock_next_and_running_semaphores();
//			pthread_mutex_unlock(&dispatcher_manager);
//
//			log_info_important(logger, "%s (id: %ld) finalizo su ejecucion!", esi->nombre, esi->id);
//			finish_esi(esi->id);
//			continue;
//		}
//
//		atomic_execution();
//		if (!valid_esi_status(esi)) {
//			continue;
//		}
//		if (send_esi_to_run(esi->id)) {
//			log_info_highlight(logger, "Se dio aviso a %s (id: %ld) para que ejecute", esi->nombre, esi->id);
//		} else {
////			unlock_next_and_running_semaphores();
//			pthread_mutex_unlock(&dispatcher_manager);
//
//			log_error(logger, "Error al avisar a %s (id: %ld) que corra", esi->nombre, esi->id);
//			finish_esi(RUNNING_ESI);
//			continue;
//		}
//
//		log_info(logger, "Esperando resultado de ejecucion de %s (id: %ld)...", esi->nombre, esi->id);
//		int result;
//		if (!wait_execution_result(RUNNING_ESI, &result)) {
////			unlock_next_and_running_semaphores();
//			pthread_mutex_unlock(&dispatcher_manager);
//
//			log_error(logger, "No se pudo recibir el resultado de ejecucion de %s (id: %ld)", esi->nombre, esi->id);
//			finish_esi(RUNNING_ESI);
//			continue;
//		}
//
//		log_info_highlight(logger, "Resultado de ejecucion de %s (id: %ld): %s", esi->nombre, esi->id, get_execution_result_description(result));
//		check_execution_error(result);
//
//		log_info(logger, "Incrementando clock de CPU (actual: %ld)", get_current_time());
//		cpu_time_incrementate();
//		log_debug(logger, "Clock de CPU incrementado (ahora es: %ld)", get_current_time());
//		borrado_de_finish();
//
////		unlock_next_and_running_semaphores();
//		pthread_mutex_unlock(&dispatcher_manager);
//	}

void init_dispatcher() {
	acquire_dispatcher();
	pthread_t orchestrator;
	if (pthread_create(&orchestrator, NULL, (void*) dispatch, NULL) < 0) {
		log_error(logger, "No se pudo iniciar el hilo del dispatcher");
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

void acquire_dispatcher() {
	pthread_mutex_lock(&dispatcher_manager);
}

//void acquire_next_and_running_semaphores() {
//	pthread_mutex_lock(&running_esi_mtx_1);
//	pthread_mutex_lock(&next_running_esi_mtx_2);
//}
//
//void unlock_next_and_running_semaphores() {
//	pthread_mutex_unlock(&next_running_esi_mtx_2);
//	pthread_mutex_unlock(&running_esi_mtx_1);
//}
