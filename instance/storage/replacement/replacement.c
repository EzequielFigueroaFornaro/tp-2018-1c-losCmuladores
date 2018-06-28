/*
 * replacement.c
 *
 *  Created on: 26 jun. 2018
 *      Author: utnso
 */

#include "replacement.h"

#include <stdlib.h>

typedef struct {
	char *key;
	int size;
} t_replacement_entry;

t_replacement_entry* _replacement_entry_create(char *key, int size);
void _replacement_entry_destroy(t_replacement_entry *entry);

t_replacement* replacement_create(t_replacement_algorithm algorithm) {
	t_replacement *replacement = (t_replacement *)malloc(sizeof(t_replacement));
	replacement -> algorithm = algorithm;
	replacement -> replacement_entries = list_create();
	return replacement;
}

void replacement_destroy(t_replacement *replacement) {
	list_destroy_and_destroy_elements(replacement -> replacement_entries, _replacement_entry_destroy);
	free(replacement);
}

void replacement_add(t_replacement *replacement, char *key, int size) {
	t_replacement_entry* entry = _replacement_entry_create(key, size);
	switch (replacement -> algorithm) {
	case CIRCULAR:
		list_add(replacement -> replacement_entries, (void *)entry);
		break;
	case LRU:
		list_add_in_index(replacement -> replacement_entries, 0, (void *)entry);
		break;
	case BSU:
		int index = 0, index_aux = 0;
		bool found = false;
		void _find_index_bsu(void *entry) {
			t_replacement_entry *replacement_entry = (t_replacement_entry *)entry;
			if (!found && size > replacement_entry -> size) {
				index = index_aux;
				found = true;
			}
			index_aux++;
		}
		list_iterate(replacement -> replacement_entries, _find_index_bsu);
		list_add_in_index(replacement -> replacement_entries, index, (void *)entry);
		break;
	}
}

char* replacement_take(t_replacement *replacement) {
	t_list *list = replacement -> replacement_entries;
	if (list_is_empty(list)) {
		return NULL;
	} else {
		t_replacement_entry* entry = (t_replacement_entry *)list_get(list, 0);
		char *key = strdup(entry -> key);
		list_remove_and_destroy_element(list, 0, _replacement_entry_destroy);
		return key;
	}
}

t_replacement_entry* _replacement_entry_create(char *key, int size) {
	t_replacement_entry* replacement_entry = (t_replacement_entry *)malloc(sizeof(t_replacement_entry));
	replacement_entry -> key = strdup(key);
	replacement_entry -> size = size;
	return replacement_entry;
}

void _replacement_entry_destroy(t_replacement_entry *entry) {
	free(entry -> key);
	free(entry);
}
