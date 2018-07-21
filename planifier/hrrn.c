/*
 * hrrn.c
 *
 *  Created on: 13 jul. 2018
 *      Author: utnso
 */

#include "hrrn.h"

void hrrn_add_esi(long esi_id) {
	pthread_mutex_lock(&ready_list_mtx_4);
	list_add_id(READY_ESI_LIST, esi_id);
	esi* esi = get_esi_by_id(esi_id);
	esi->ultima_entrada_a_ready = get_current_time();
	esi->estimacion_ultima_rafaga = estimate_next_cpu_burst(esi);
	log_info_highlight(logger,
			"--HRRN--Se agrego el %s a la cola de listos. tiempo de entrada: %ld, ultima refaga: %2.2f",
			esi->nombre, esi->ultima_entrada_a_ready,
			esi->estimacion_ultima_rafaga);
	pthread_mutex_unlock(&ready_list_mtx_4);
}

void hrrn_block_esi(long block_esi_id) {
	pthread_mutex_lock(&running_esi_mtx_1);
	pthread_mutex_lock(&next_running_esi_mtx_2);
	pthread_mutex_lock(&blocked_list_mtx_3);
	pthread_mutex_lock(&ready_list_mtx_4);

	if (RUNNING_ESI == block_esi_id) {
		hrrn_replan();
	} else {
		list_remove_esi(READY_ESI_LIST, block_esi_id);
	}

	pthread_mutex_unlock(&ready_list_mtx_4);
	list_add_id(BLOCKED_ESI_LIST, block_esi_id);
	pthread_mutex_unlock(&blocked_list_mtx_3);
	pthread_mutex_unlock(&next_running_esi_mtx_2);
	pthread_mutex_unlock(&running_esi_mtx_1);
}

float response_ratio(long* esi_id) {
	esi* esi = get_esi_by_id(*esi_id);
	float esi_estimated_burst = esi->estimacion_ultima_rafaga;
	long time_when_it_would_execute = get_current_time() + 1;
	log_debug(logger, "--HRRN-- %s: tiempo en el que seria ejecutado: %ld", esi->nombre, time_when_it_would_execute);
	int time_waiting = time_when_it_would_execute - esi->ultima_entrada_a_ready;
	float response_ratio = (time_waiting + esi_estimated_burst) / esi_estimated_burst;
	log_info_highlight(logger, "--HRRN-- %s: S = %2.2f, W = %d, RR = %2.2f", esi->nombre, esi_estimated_burst, time_waiting, response_ratio);
	return response_ratio;
}

void hrrn_replan() {
	log_info(logger, "Replaning...");

	bool higher_response_ratio(long* esi_id, long* other_esi_id) {
		return response_ratio(other_esi_id) >= response_ratio(esi_id);
	}
	list_sort(READY_ESI_LIST, (void*) higher_response_ratio);
	log_info_important(logger, "--HRRN-- Orden de la cola de listos: [%s]", list_join(READY_ESI_LIST));

	long* next_esi = list_remove(READY_ESI_LIST, 0);
	if (next_esi == NULL) {
		NEXT_RUNNING_ESI = 0;
		READY_ESI_LIST = list_create();
	} else {
		NEXT_RUNNING_ESI = *next_esi;
		log_debug(logger, "Next ESI to run is ESI%ld", NEXT_RUNNING_ESI);
	}
}

void hrrn_finish_esi() {
	pthread_mutex_lock(&next_running_esi_mtx_2);
	pthread_mutex_lock(&ready_list_mtx_4);
	hrrn_replan();
	pthread_mutex_unlock(&ready_list_mtx_4);
	pthread_mutex_unlock(&next_running_esi_mtx_2);
}
