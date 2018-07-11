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
#include "commons/string.h"

typedef struct {
	long entries_size;
	long entries_quantity;
}__attribute((packed)) t_instance_configuration;

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

typedef enum { COORDINATOR, PLANIFIER, INSTANCE, ISE } module_type;

typedef int message_type;
extern message_type MODULE_CONNECTED;
extern message_type CONNECTION_SUCCESS;
extern message_type EXECUTION_RESULT;
extern message_type PROCESS_SENTENCE;
extern message_type ISE_STOP;
extern message_type ISE_EXECUTE;
extern message_type ISE_KILL;
extern message_type KEY_INFO_REQUEST;
extern message_type GET_INSTANCE;
extern message_type GET_KEY_VALUE;
extern message_type CALCULATE_INSTANCE;
extern message_type KEY_INFO_REQUEST_FINISHED;

t_sentence* sentence_create();

t_sentence* sentence_create_with(int operation_id, char* key, char* value);

void sentence_destroy(t_sentence* sentence);

bool is_valid_operation(int operation);

t_buffer serialize_operation_resource_request(int operation_id, char* key, long ise_id);

//Devuelve un buffer y su size a partir de una sentencia.
t_buffer serialize_sentence(t_sentence* sentence); // TODO [Lu] no deberia devolver un puntero a t_buffer?

//Destruye el buffer correspondiente.
void destroy_buffer(t_buffer buffer);

/* Copia el valor y actualiza la posicion de memoria para copiar otro valor */
void concat_value(void** mem_address, void* value, int size_of_value);

/* Copia el tamaño del string junto al contenido y actualiza la posicion de memoria para copiar otro valor */
void concat_string(void** mem_address, void* string, int string_length);

char* get_operation_as_string(int operation_id);

char* sentence_to_string(t_sentence* sentence);

char* get_operation_as_string(int operation_id);

#endif /* TYPES_H_ */
