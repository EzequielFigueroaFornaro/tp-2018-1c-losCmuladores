/*
 * keymap.h
 *
 *  Created on: 27 may. 2018
 *      Author: utnso
 */

#ifndef STORAGE_ENTRY_TABLE_H_
#define STORAGE_ENTRY_TABLE_H_

#include <commons/collections/dictionary.h>
#include <stddef.h>

#include "availability.h"

typedef struct {
	int index;
	int length;
} t_entry;

typedef struct {
	t_dictionary *entries;
	t_availability *availability;
	char *data;
	int max_entries;
	size_t entry_size;
} t_entry_table;

t_entry_table *entry_table_create(int max_entries, size_t entry_size);

void entry_table_destroy(t_entry_table* entry_table);

int entry_table_put(t_entry_table* entry_table, char *key, char *value);

char* entry_table_get(t_entry_table* entry_table, char *key);

void entry_table_remove(t_entry_table * entry_table, char *key);

int entry_table_store(t_entry_table * entry_table, char* mount_path, char *key);

int entry_table_load(t_entry_table * entry_table, char* mount_path, char *key);

t_entry_table *entry_table_create(int max_entries, size_t entry_size);

#endif /* STORAGE_ENTRY_TABLE_H_ */
