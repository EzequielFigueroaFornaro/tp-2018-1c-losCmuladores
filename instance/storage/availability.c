/*
 * availability.c
 *
 *  Created on: 27 may. 2018
 *      Author: utnso
 */

#include "availability.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

int calculate_char_size(int entries_count) {
	int size = entries_count / CHAR_BIT;
	if (entries_count % CHAR_BIT > 0) {
		return size + 1;
	} else {
		return size;
	}
}

bool _availability_test_entry(t_availability *availability, int index) {
	return bitarray_test_bit(availability -> bitarray, index);
}

t_availability *availability_create(int entries_count) {
	int size = calculate_char_size(entries_count);
	char *bitarray = (char *) malloc(sizeof(char) * size);
	memset(bitarray, 0, size);
	t_bitarray *t_bitarray = bitarray_create_with_mode(bitarray, size, LSB_FIRST);

	t_availability *availability = (t_availability *) malloc(sizeof(t_availability));
	availability -> bitarray = t_bitarray;
	availability -> max_entries = entries_count;
	return availability;
}

void availability_destroy(t_availability *availability) {
	// bug de la commons, no libera
	free(availability -> bitarray -> bitarray);
	bitarray_destroy(availability -> bitarray);
	free(availability);
}

int availability_get_free_entries_count(t_availability *availability) {
	return availability -> max_entries - availability_get_taken_entries_count(availability);
}

int availability_get_taken_entries_count(t_availability *availability) {
	int count = 0;
	for (int i = 0; i < availability->max_entries; ++i) {
		if (bitarray_test_bit(availability -> bitarray, i)) {
			count++;
		}
	}
	return count;
}

bool availability_is_continous(t_availability *availability, int index, int continuous_size) {
	int max = index + continuous_size;
	for (int i = index; i < max; ++i) {
		if (bitarray_test_bit(availability->bitarray, i)) {
			return false;
		}
	}
	return true;
}

int availability_find_free_countinuous_index(t_availability *availability, int continuous_size) {
	int max = availability->max_entries - continuous_size;
	for (int i = 0; i <= max; ++i) {
		if (availability_is_continous(availability, i, continuous_size)) {
			return i;
		}
	}
	return -1;
}

bool availability_has_free_countinuous_space(t_availability *availability, int continuous_size) {
	return availability_find_free_countinuous_index(availability, continuous_size) != -1;
}

void availability_set_space(t_availability *availability, int index, int continuous_size, bool value) {
	int max = index + continuous_size;
	for (int i = index; i < max; ++i) {
		if (value) {
			bitarray_set_bit(availability -> bitarray, i);
		} else {
			bitarray_clean_bit(availability -> bitarray, i);
		}
	}
}

void availability_take_space(t_availability *availability, int index, int continuous_size) {
	availability_set_space(availability, index, continuous_size, true);
}

void availability_free_space(t_availability *availability, int index, int continuous_size) {
	availability_set_space(availability, index, continuous_size, false);
}
