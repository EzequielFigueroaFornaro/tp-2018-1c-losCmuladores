/*
 * planifier_structures.c
 *
 *  Created on: 20 jun. 2018
 *      Author: utnso
 */
#include "planifier_structures.h"

t_queue* get_blocked_queue(char* resource) {
	if (!dictionary_has_key(esis_bloqueados_por_recurso, resource)) {
		return queue_create();
	}
	return dictionary_get(esis_bloqueados_por_recurso, resource);
}

void add_to_blocked_queue(char* resource, long esi_id) {
	pthread_mutex_lock(&map_boqueados);

	t_queue* blocked_esis = get_blocked_queue(resource);
	queue_push(blocked_esis, &esi_id);
	dictionary_put(esis_bloqueados_por_recurso, resource, blocked_esis);

	pthread_mutex_unlock(&map_boqueados);
}

char* blocked_queue_to_string(char* resource) {
	pthread_mutex_lock(&map_boqueados);

	if (esis_bloqueados_por_recurso != NULL) {
		t_queue* esis = dictionary_get(esis_bloqueados_por_recurso, resource);
		if (esis != NULL) {
			char* buffer = string_new();
			void to_string(long* esi_id) {
				string_append_with_format(&buffer, "ESI%ld\n", *esi_id);
			}
			list_iterate(esis->elements, (void*) to_string);

			pthread_mutex_unlock(&map_boqueados);
			return buffer;
		}
	}
	pthread_mutex_unlock(&map_boqueados);
	return "";
}
