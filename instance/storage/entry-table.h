/*
 * keymap.h
 *
 *  Created on: 27 may. 2018
 *      Author: utnso
 */

#ifndef STORAGE_ENTRY_TABLE_H_
#define STORAGE_ENTRY_TABLE_H_

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stddef.h>

#include "replacement/replacement.h"
#include "availability/availability.h"

typedef struct {
	int index;
	int length;
} t_entry;

typedef struct {
	// t_entry dictionary
	t_dictionary *entries;
	t_availability *availability;
	t_replacement *replacement;
	char *data;
	int max_entries;
	size_t entry_size;
} t_entry_table;

t_entry_table *entry_table_create(int max_entries, size_t entry_size, t_replacement_algorithm replacement_algorithm);

void entry_table_destroy(t_entry_table* entry_table);

/**
 * Inserta una nueva clave en la tabla
 * return
 *       -1 Error generico
 *       -2 Necesita compactar
 */
int entry_table_put(t_entry_table* entry_table, char *key, char *value);

/**
 * Obtiene el valor de la entrada
 */
char* entry_table_get(t_entry_table* entry_table, char *key);

/**
 * Elimina la entrada de la tabla
 */
void entry_table_remove(t_entry_table * entry_table, char *key);

/**
 * Almacena el valor de la entrada en disco
 */
int entry_table_store(t_entry_table * entry_table, char* mounting_path, char *key);

/**
 * Carga el valor del archivo en disco a la tabla de entradas
 */
int entry_table_load(t_entry_table * entry_table, char* mounting_path, char *key);

/**
 * determina si hay suficiente espacio continuo para poder almacenar el valor
 */
bool entry_table_can_put(t_entry_table* entry_table, char *value);

/**
 * Determina si hay suficiente espacio poder almacenar el valor, no necesariamente el espacio es continuo
 * Usado para compactar
 */
bool entry_table_enough_free_entries(t_entry_table* entry_table, char *value);

/**
 * Determina si hay entradas atomicas
 * Usado para reemplazar entradas
 */
bool entry_table_has_atomic_entries(t_entry_table* entry_table);

/**
 * Compacta las entradas
 */
void entry_table_compact(t_entry_table * entry_table);

#endif /* STORAGE_ENTRY_TABLE_H_ */
