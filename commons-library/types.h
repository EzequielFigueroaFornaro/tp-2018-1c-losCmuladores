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
#include <stdbool.h>
#include <string.h>

typedef struct {
	int operation_id;
	char* key;
	char* value;
} t_sentence;

typedef struct {
	void* buffer_content;
	int size;
} t_buffer ;

enum operations {
	GET_SENTENCE = 600,
	SET_SENTENCE = 601,
	STORE_SENTENCE = 602
} operation ;

bool is_valid_operation(int operation);

//Devuelve un buffer y su size a partir de una sentencia.
t_buffer serialize_sentence(t_sentence* sentence); // TODO [Lu] no deberia devolver un puntero a t_buffer?

//Destruye el buffer correspondiente.
void destroy_buffer(t_buffer buffer);

/* Copia el valor y actualiza la posicion de memoria para copiar otro valor */
void concat_value(void** mem_address, void* value, int size_of_value);

/* Copia el tamaño del string junto al contenido y actualiza la posicion de memoria para copiar otro valor */
void concat_string(void** mem_address, void* string, int string_length);

#endif /* TYPES_H_ */
