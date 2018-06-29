/*
 * replacement.h
 *
 *  Created on: 26 jun. 2018
 *      Author: utnso
 */

#ifndef STORAGE_REPLACEMENT_REPLACEMENT_H_
#define STORAGE_REPLACEMENT_REPLACEMENT_H_

#include <commons/collections/list.h>

typedef enum {
	CIRCULAR = 0,
	LRU = 1,
	BSU = 2
} t_replacement_algorithm;

typedef struct {
	t_replacement_algorithm algorithm;
	t_list *replacement_entries;
} t_replacement;

t_replacement* replacement_create(t_replacement_algorithm algorithm);

void replacement_destroy(t_replacement *replacement);

void replacement_add(t_replacement *replacement, char *key, int size);

void replacement_remove(t_replacement *replacement, char *key);

char *replacement_take(t_replacement *replacement);

bool replacement_is_empty(t_replacement *replacement);

#endif /* STORAGE_REPLACEMENT_REPLACEMENT_H_ */
