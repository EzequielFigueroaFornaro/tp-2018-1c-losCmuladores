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

t_queue* get_all_waiting_for_resource(char* resource) {
	if (!dictionary_has_key(esis_bloqueados_por_recurso, resource)) {
		return queue_create();
	}
	return dictionary_get(esis_bloqueados_por_recurso, resource);
}

void make_wait_for_resource(long esi_id, char* resource) {
	pthread_mutex_lock(&blocked_by_resource_map_mtx);

	t_queue* blocked_esis = get_all_waiting_for_resource(resource);
	queue_push_id(blocked_esis, esi_id);
	dictionary_put(esis_bloqueados_por_recurso, resource, blocked_esis);

	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
}

char* get_all_waiting_for_resource_as_string(char* resource) {
	pthread_mutex_lock(&blocked_by_resource_map_mtx);

	if (esis_bloqueados_por_recurso != NULL) {
		t_queue* esis_blocked = get_all_waiting_for_resource(resource);
		if (!queue_is_empty(esis_blocked)) {
			char* buffer = string_new();

			void to_string(long* esi_id) {
				string_append_with_format(&buffer, "ESI%ld\n", *esi_id);
			}
			list_iterate(esis_blocked->elements, (void*) to_string);

			pthread_mutex_unlock(&blocked_by_resource_map_mtx);
			return buffer;
		} else {
			queue_destroy(esis_blocked);
		}
	}
	pthread_mutex_unlock(&blocked_by_resource_map_mtx);
	return "";
}
