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
	int time_waiting = get_current_time() - esi->ultima_entrada_a_ready;
	float response_ratio = (time_waiting + esi_estimated_burst) / esi_estimated_burst;
	log_debug(logger, "ESI%ld: S = %2.5f, W = %d", *esi_id, esi_estimated_burst, time_waiting);
	log_debug(logger, "ESI%ld's response ratio is %2.5f", *esi_id, response_ratio);
	return response_ratio;
}

void hrrn_replan() {
	log_info(logger, "Replaning...");

	bool higher_response_ratio(long* esi_id, long* other_esi_id) {
		return response_ratio(esi_id) >= response_ratio(other_esi_id);
	}
	list_sort(READY_ESI_LIST, (void*) higher_response_ratio);

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
