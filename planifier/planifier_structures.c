/*
 * planifier_structures.c
 *
 *  Created on: 20 jun. 2018
 *      Author: utnso
 */
#include "planifier_structures.h"

long id_as_long(char* id) {
	return atol(id);
}

char* id_to_string(long id) {
	return string_from_format("%ld", id);
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
	long* esi_id = malloc(sizeof(long));
	*esi_id = id;
	queue_push(queue, esi_id);
}

void list_add_id(t_list* list, long id) {
	long* esi_id = malloc(sizeof(long));
	*esi_id = id;
	list_add(list, esi_id);
}

void dictionary_put_id(t_dictionary* map, char* key, long id) {
	long* esi_id = malloc(sizeof(long));
	*esi_id = id;
	dictionary_put_posta(map, key, esi_id);
}

char* esi_status_to_string(estado status) {
	switch(status) {
	case NUEVO: return "NUEVO";
	case BLOQUEADO: return "BLOQUEADO";
	case CORRIENDO: return "CORRIENDO";
	case FINALIZADO: return "FINALIZADO";
	case DESBLOQUEADO: return "DESBLOQUEADO";
	}
	return "DESCONOCIDO";
}

t_list* get_resources_taken_by_esi(long esi_id) {
	t_list* resources = list_create();

	void find_resource(char* key, long* value) {
		if (esi_id == *value) {
			list_add(resources, key);
		}
	}
	pthread_mutex_lock(&blocked_resources_map_mtx);
	if (!dictionary_is_empty(recurso_tomado_por_esi)) {
		dictionary_iterator(recurso_tomado_por_esi, (void*) find_resource);
	}
	pthread_mutex_unlock(&blocked_resources_map_mtx);
	return resources;
}

bool is_resource_taken_by_esi(long esi_id, char* resource) {
    long *esi_id_que_lo_tomo = malloc(sizeof(long));
    pthread_mutex_lock(&blocked_resources_map_mtx);
    if (!dictionary_is_empty(recurso_tomado_por_esi)) {
        esi_id_que_lo_tomo =(long*) dictionary_get(recurso_tomado_por_esi, resource);
    }
    pthread_mutex_unlock(&blocked_resources_map_mtx);
    bool taken = *esi_id_que_lo_tomo != 0 && *esi_id_que_lo_tomo == esi_id;
    free(esi_id_que_lo_tomo);
    return taken;
    /*t_list* resources = get_resources_taken_by_esi(esi_id);

	bool contains(char* r) {
		return strcmp(r, resource) == 0;
	}
	bool taken = list_any_satisfy(resources, (void*) contains);
	list_destroy(resources);
	return taken;*/
}

char* esi_to_string(esi* esi) {
	return string_from_format("{ id: %ld, "
							  "estado: %s, "
							  "recurso_que_lo_bloquea: %s, "
							  "instruccion_actual: %d/%d }",
							  esi->id,
							  esi_status_to_string(esi->estado),
//							  get_resource_taken_by_esi(esi->id),
							  esi->blocking_resource,
							  esi->instruction_pointer, esi->cantidad_de_instrucciones);

}

char* esis_to_string() {
	char* buffer = string_new();
	void to_string(char* esi_id, esi* esi) {
		string_append_with_format(&buffer, "\n%s", esi_to_string(esi));
	}
	dictionary_iterator(esi_map, (void*)to_string);
	return buffer;
}

void list_remove_esi(t_list* list, long id) {
	bool equals_esi(long* esi_id) {
		return *esi_id == id;
	}
	list_remove_by_condition(list, (void*) equals_esi);
}

esi* get_esi_by_id(long esi_id) {
	esi* esi;
	pthread_mutex_lock(&esi_map_mtx_6);
	esi = dictionary_get(esi_map, id_to_string(esi_id));
	pthread_mutex_unlock(&esi_map_mtx_6);
	return esi;
}

bool string_is_blank(char* string) {
	return string == NULL || string_is_empty(string);
}
