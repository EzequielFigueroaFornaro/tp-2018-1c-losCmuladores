/*
 * entrymap.c
 *
 *  Created on: 30 may. 2018
 *      Author: utnso
 */

#include "entry-table.h"

#include <commons/string.h>
#include <stdlib.h>
#include <string.h>

#include "file/file-system.h"
#include "logging.h"

char* _make_full_file_name(char *mount_path, char *key);
int _calculate_value_length(char *value);
int _calculate_value_length_entries_count(t_entry_table * table, int value_len);
int _calculate_value_entries_count(t_entry_table * table, char *value);
char* _calculate_data_address(t_entry_table * table, int index);
void _copy_value_in_data(t_entry_table * table, char *value, int index_data);
char* _get_value_from_data(t_entry_table * table, int index_data);
void _add_entry_in_table_dictionary(t_entry_table * table, char *key, char *value, int index);
bool _is_atomic(t_entry_table *entry_table, char *key);
int _entry_table_try_put(t_entry_table * table, char *key, char *value);
bool _entry_table_has_key(t_entry_table * table, char *key);


t_entry_table *entry_table_create(int max_entries, size_t entry_size, t_replacement_algorithm algorithm) {
	size_t data_size = max_entries * entry_size;

	t_dictionary *entries = dictionary_create();
	t_availability *availability = availability_create(max_entries);
	char *data = (char*) malloc(sizeof(char) * data_size);

	t_entry_table *table = (t_entry_table*) malloc(sizeof(t_entry_table));
	table->entries = entries;
	table->availability = availability;
	table->replacement = replacement_create(algorithm);
	table->data = data;
	table->max_entries = max_entries;
	table->entry_size = entry_size;

	return table;
}

void entry_table_destroy(t_entry_table* entry_table) {
	if (NULL != entry_table) {
		availability_destroy(entry_table->availability);
		dictionary_destroy_and_destroy_elements(entry_table->entries, free);
		replacement_destroy(entry_table->replacement);
		free(entry_table->data);
		free(entry_table);
	}
}

int entry_table_put(t_entry_table * table, char *key, char *value) {
	bool retry = true;
	int result = -1;
	do {
		if (_entry_table_has_key(table, key)) {
			entry_table_remove(table, key);
		}
		int try_put_result = _entry_table_try_put(table, key, value);
		if (try_put_result < 0) {
			char *key_to_replace = replacement_take(table->replacement);
			if (key_to_replace == NULL) {
				result = -1;
				retry = false;
			} else {
				entry_table_remove(table, key_to_replace);
			}
		} else {
			result = try_put_result;
			retry = false;
		}
	} while(retry);
	return result;
}

char* entry_table_get(t_entry_table * entry_table, char *key) {
	t_dictionary* entries = entry_table->entries;
	if (dictionary_has_key(entries, key)) {
		t_entry *entry = (t_entry*) dictionary_get(entries, key);
		return _get_value_from_data(entry_table, entry->index);
	} else {
		return NULL;
	}
}

void entry_table_remove(t_entry_table * entry_table, char *key) {
	t_entry* entry = (t_entry*)dictionary_get(entry_table->entries, key);
	int entries_count = _calculate_value_length_entries_count(entry_table, entry->length);
	availability_free_space(entry_table->availability, entry->index, entries_count);
	dictionary_remove_and_destroy(entry_table->entries, key, free);

	replacement_remove(entry_table->replacement, key);
}

int entry_table_store(t_entry_table * entry_table, char* mount_path, char *key) {
	char *file_name = _make_full_file_name(mount_path, key);
	char * value = entry_table_get(entry_table, key);
	int result = -1;
	if (NULL != value) {
		entry_table_remove(entry_table, key);
		result = file_system_save(file_name, value);
	}
	free(file_name);
	free(value);
	return result;
}

int entry_table_load(t_entry_table * entry_table, char* mount_path, char *key) {
	char *file_name = _make_full_file_name(mount_path, key);
	char *value = file_system_read(file_name);
	return entry_table_put(entry_table, key, value);
}

bool entry_table_can_put(t_entry_table* entry_table, char *value) {
	int count = _calculate_value_entries_count(entry_table, value);
	return availability_has_free_countinuous_space(entry_table->availability, count);
}

bool entry_table_enough_free_entries(t_entry_table* entry_table, char *value) {
	int count_needed = _calculate_value_entries_count(entry_table, value);
	int free_entries = availability_get_free_entries_count(entry_table->availability);
	return free_entries >= count_needed;
}

bool entry_table_has_atomic_entries(t_entry_table* entry_table) {
	return !replacement_is_empty(entry_table->replacement);
}





char* _make_full_file_name(char *mount_path, char *key) {
	char *file_name = string_duplicate(mount_path);
	string_append(&file_name, key);
	return file_name;
}

int _calculate_value_length(char *value) {
	return strlen(value) + 1;
}

int _calculate_value_length_entries_count(t_entry_table * table, int value_len) {
	int entries_count = value_len / table->entry_size;
	if (value_len % table->entry_size > 0) {
		entries_count++;
	}
	return entries_count;
}

int _calculate_value_entries_count(t_entry_table * table, char *value) {
	int value_len = _calculate_value_length(value);

	return _calculate_value_length_entries_count(table, value_len);
}

char* _calculate_data_address(t_entry_table * table, int index) {
	int offset = sizeof(char) * table->entry_size * index;
	return table->data + offset;
}

void _copy_value_in_data(t_entry_table * table, char *value, int index_data) {
	char *data_address = _calculate_data_address(table, index_data);
	strcpy(data_address, value);
}

char* _get_value_from_data(t_entry_table * table, int index_data) {
	char *data_address = _calculate_data_address(table, index_data);
	return strdup(data_address);
}

void _add_entry_in_table_dictionary(t_entry_table * table, char *key, char *value, int index) {
	t_entry *entry = (t_entry*) malloc(sizeof(t_entry));
	entry->index = index;
	entry->length = _calculate_value_length(value);

	dictionary_put(table->entries, key, (void*)entry);
}

bool _is_atomic(t_entry_table *entry_table, char *key) {
	int length = ((t_entry *)dictionary_get(entry_table->entries, key))->length;
	return length <= entry_table->entry_size;
}

int _entry_table_try_put(t_entry_table * table, char *key, char *value) {
	int entries_needed = _calculate_value_entries_count(table, value);
	int index = availability_find_free_countinuous_index(table->availability, entries_needed);
	if (index >= 0) {
		availability_take_space(table->availability, index, entries_needed);
		_copy_value_in_data(table, value, index);
		_add_entry_in_table_dictionary(table, key, value, index);
		if (_is_atomic(table, key)) {
			replacement_add(table->replacement, key, _calculate_value_length(value));
		}
		return index;
	} else {
		return -1;
	}
}

bool _entry_table_has_key(t_entry_table * table, char *key) {
	return dictionary_has_key(table->entries, key);
}

