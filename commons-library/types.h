/*
 * types.h
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	int operation_id;
	char* key;
	char* value;
} t_sentence;

typedef struct {
	int operation_id;
	int key_length;
	int value_length;
} __attribute((packed)) t_sentence_header;

typedef struct {
	void* buffer_content;
	int size;
} t_buffer ;

enum operations {
	GET_SENTENCE = 600,
	SET_SENTENCE = 601,
	STORE_SENTENCE = 602
} operation ;

//Devuelve un buffer y su size a partir de una sentencia.
t_buffer* serialize_sentence(t_sentence* sentence);

//Destruye el buffer correspondiente.
void destroy_buffer(t_buffer* buffer);

#endif /* TYPES_H_ */
