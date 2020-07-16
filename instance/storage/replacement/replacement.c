/*
 * replacement.c
 *
 *  Created on: 26 jun. 2018
 *      Author: utnso
 */

#include "replacement.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "logging.h"
#include <commons/string.h>

typedef struct {
	char *key;
	int size;
} t_replacement_entry;

t_replacement_entry* _replacement_entry_create(char *key, int size);
void _replacement_entry_destroy(void *entry);
int _find_index_to_insert_bsu(t_replacement *replacement, int size);
int _replacement_size(t_replacement *replacement);
bool _replacement_contains(t_replacement *replacement, char *key);

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
		if (!_replacement_contains(replacement, key)) {
			list_add(replacement -> replacement_entries, (void *)entry);
		} else {
			_replacement_entry_destroy(entry);
		}
		break;
	case LRU:
		replacement_remove(replacement, key);
		int lru_index = list_size(replacement -> replacement_entries);
		list_add_in_index(replacement -> replacement_entries, lru_index, (void *)entry);
		break;
	case BSU:
		replacement_remove(replacement, key);
		int bsu_index = _find_index_to_insert_bsu(replacement, size);
		list_add_in_index(replacement -> replacement_entries, bsu_index, (void *)entry);
		break;
	}
}

void replacement_remove(t_replacement *replacement, char *key) {
	bool _equals_key(void *entry) {
		t_replacement_entry *item = (t_replacement_entry *)entry;
		return strcmp(key, item -> key) == 0;
	}

	list_remove_and_destroy_by_condition(replacement -> replacement_entries, _equals_key, _replacement_entry_destroy);
}

char* replacement_take(t_replacement *replacement) {
	t_list *list = replacement -> replacement_entries;
	if (list_is_empty(list)) {
		return NULL;
	} else {
		t_replacement_entry* entry = (t_replacement_entry *)list_get(list, 0);
		return strdup(entry -> key);
	}
}

bool replacement_is_empty(t_replacement *replacement) {
	return list_is_empty(replacement -> replacement_entries);
}

void replacement_log_debug(t_replacement *replacement) {
	char *string = strdup("Replacement list is: ");
	char *format;
	int count = list_size(replacement->replacement_entries);
	for (int i = 0; i < count; ++i) {
		t_replacement_entry * item = (t_replacement_entry *)list_get(replacement->replacement_entries, i);
		format = string_from_format("key %s %d, ", item->key, item->size);
		string_append(&string, format);
		free(format);
	}
	log_debug(logger, string);
	free(string);
}




t_replacement_entry* _replacement_entry_create(char *key, int size) {
	t_replacement_entry* replacement_entry = (t_replacement_entry *)malloc(sizeof(t_replacement_entry));
	replacement_entry -> key = strdup(key);
	replacement_entry -> size = size;
	return replacement_entry;
}

void _replacement_entry_destroy(void *entry) {
	t_replacement_entry *element = (t_replacement_entry*) entry;
	free(element -> key);
	free(element);
}

int _find_index_to_insert_bsu(t_replacement *replacement, int size) {
	int index = _replacement_size(replacement), index_aux = 0;
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
	return index;
}

int _replacement_size(t_replacement *replacement) {
	return list_size(replacement -> replacement_entries);
}

bool _replacement_contains(t_replacement *replacement, char *key) {
	bool _equals_key(void *entry) {
		t_replacement_entry *item = (t_replacement_entry *)entry;
		return strcmp(key, item -> key) == 0;
	}

	return list_any_satisfy(replacement -> replacement_entries, _equals_key);
}


