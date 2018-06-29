/*
 * planifier_structures.c
 *
 *  Created on: 20 jun. 2018
 *      Author: utnso
 */
#include "planifier_structures.h"

long id_as_long(char* esi_id) {
	return atol(esi_id);
}

char* list_join(t_list* list) {
	char* buffer = string_new();
	int size = list_size(list);
	for(int i=0; i < size; i++) {
		long* id = list_get(list, i);
		string_append_with_format(&buffer, "%ld", *id);

		if ((i + 1) < size) {
			string_append_with_format(&buffer, ",");
		}
	}
	return buffer;
}

void queue_push_id(t_queue* queue, long id) {
	long* esi_id = malloc(sizeof(long)); // TODO [Lu] free
	*esi_id = id;
	queue_push(queue, esi_id);
}

void list_add_id(t_list* list, long id) {
	long* esi_id = malloc(sizeof(long)); // TODO [Lu] free
	*esi_id = id;
	list_add(list, esi_id);
}

char* esis_to_string() {
	char* buffer = string_new();
	void to_string(char* esi_id, esi* esi) {
		string_append_with_format(&buffer, "\n\t\t\t\t\t\tESI%ld -> status: %d", esi->id, esi->estado);
	}
	dictionary_iterator(esi_map, (void*)to_string);
	return buffer;
}

void list_remove_esi(t_list* list, esi* esi) {
	pthread_mutex_lock(&esi_map_mtx);

	bool equals_esi(long* esi_id) {
		return *esi_id == esi->id;
	}
	list_remove_by_condition(list, (void*) equals_esi);
	pthread_mutex_unlock(&esi_map_mtx);
}
