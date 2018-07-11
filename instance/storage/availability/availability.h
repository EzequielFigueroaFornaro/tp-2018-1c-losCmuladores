/*
 * availability.h
 *
 *  Created on: 27 may. 2018
 *      Author: utnso
 */

#ifndef STORAGE_AVAILABILITY_H_
#define STORAGE_AVAILABILITY_H_

#include <commons/bitarray.h>
#include <stddef.h>

typedef struct {
	t_bitarray *bitarray;
	int max_entries;
} t_availability;

t_availability *availability_create(int entries_count);

void availability_destroy(t_availability *availability);

int availability_get_free_entries_count(t_availability *availability);

int availability_get_taken_entries_count(t_availability *availability);

int availability_find_free_countinuous_index(t_availability *availability, int continuous_size);

bool availability_has_free_countinuous_space(t_availability *availability, int continuous_size);

void availability_take_space(t_availability *availability, int index, int continuous_size);

void availability_free_space(t_availability *availability, int index, int continuous_size);

void availability_log_debug(t_availability *availability);

bool availability_need_compaction(t_availability *availability, int entries_needed);

#endif /* STORAGE_AVAILABILITY_H_ */
