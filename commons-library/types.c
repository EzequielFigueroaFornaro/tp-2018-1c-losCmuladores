/*
 * types.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#include "types.h"

message_type MODULE_CONNECTED = 100;
message_type CONNECTION_SUCCESS = 101;
message_type EXECUTION_RESULT = 200;
message_type PROCESS_SENTENCE = 201;
message_type START_COMPACTION = 202;
message_type ISE_STOP = 300;
message_type ISE_EXECUTE = 301;
message_type ISE_KILL = 302;
message_type KEY_INFO_REQUEST = 400;
message_type GET_INSTANCE = 401;
message_type GET_KEY_VALUE = 402;
message_type CALCULATE_INSTANCE = 403;
message_type KEY_INFO_REQUEST_FINISHED = 404;


void dictionary_put_posta(t_dictionary *self, char *key, void *data) {
	void * element = dictionary_get(self, key);
	if(element != NULL) {
		dictionary_remove(self, key);
	}
	dictionary_put(self, key, data);
}

t_buffer serialize_operation_resource_request(int operation_id, char* key, long ise_id){
	int key_length = strlen(key) + 1;
	int message_size = sizeof(int) + sizeof(int) + key_length + sizeof(long);

	void* buffer = malloc(message_size);
	void* offset = buffer;

	concat_value(&offset, &(operation_id), sizeof(int));
	concat_string(&offset, key, key_length);
	concat_value(&offset, &ise_id, sizeof(long));

	t_buffer buffer_struct;
	buffer_struct.buffer_content = buffer;
	buffer_struct.size = message_size;

	return buffer_struct;
}

t_sentence* sentence_create() {
	t_sentence* sentence = malloc(sizeof(t_sentence));
	sentence->operation_id = 0;
	sentence->key = NULL;
	sentence->value = NULL;
	return sentence;
}

t_planifier_sentence* planifier_sentence_create() {
	t_planifier_sentence* sentence = malloc(sizeof(t_planifier_sentence));
	sentence->operation_id = 0;
	sentence->resource = NULL;
	sentence->esi_id = 0;
	return sentence;
}

t_sentence* sentence_create_with(int operation_id, char* key, char* value) {
	t_sentence* sentence = sentence_create();
	sentence -> operation_id = operation_id;
	sentence -> key = malloc(strlen(key) + 1);
	strcpy(sentence -> key, key);
	sentence -> value = malloc(strlen(value) + 1);
	strcpy(sentence -> value, value);
	return sentence;
}

void sentence_destroy(t_sentence* sentence) {
	if (NULL != sentence) {
		free(sentence->key);
		free(sentence->value);
		free(sentence);
	}
}

void planifier_sentence_destroy(t_planifier_sentence* sentence) {
    if (NULL != sentence) {
        free(sentence->resource);
        free(sentence);
    }
}

t_buffer serialize_sentence(t_sentence* sentence){
	int operation_length = sizeof(sentence -> operation_id);
	int key_length = strlen(sentence -> key) + 1;
	int value_length = strlen(sentence -> value) + 1;

	// message_type:PROCESS_SENTENCE + operation_id + key_length + value_int + value_length
	int message_size = sizeof(message_type) + sizeof(int) + sizeof(int)
			+ key_length + sizeof(int) + value_length;
	void* buffer = malloc(message_size);
	void* offset = buffer;
	concat_value(&offset, &PROCESS_SENTENCE, sizeof(message_type));
	concat_value(&offset, &(sentence -> operation_id), operation_length);
	concat_string(&offset, sentence -> key, key_length);
	concat_string(&offset, sentence -> value, value_length);

	t_buffer buffer_struct;
	buffer_struct.buffer_content = buffer;
	buffer_struct.size = message_size;

	return buffer_struct;
}

void destroy_buffer(t_buffer buffer){
	free(buffer.buffer_content);
}

bool is_valid_operation(int operation) {
	return operation == GET_SENTENCE ||
			operation == SET_SENTENCE ||
			operation == STORE_SENTENCE;
}

void concat_value(void** mem_address, void* value, int size_of_value) {
	memcpy(*mem_address, value, size_of_value);
	*mem_address = *mem_address + size_of_value;
}

void concat_string(void** mem_address, void* string, int string_length) {
	void* offset = *mem_address;
	concat_value(&offset, &string_length, sizeof(string_length));
	concat_value(&offset, string, string_length);
	*mem_address = offset;
}

char* get_operation_as_string(int operation_id) {
	switch(operation_id) {
		case GET_SENTENCE: return "GET";
		case SET_SENTENCE: return "SET";
		case STORE_SENTENCE: return "STORE";
		default: return NULL;
	}
}

//TODO estos returns pierden memoria segun valgrind.
char* sentence_to_string(t_sentence* sentence) {
	if (strcmp(sentence->value, "") == 0) {
		return string_from_format("{ %s %s }",
				get_operation_as_string(sentence->operation_id), sentence->key);
	}
	return string_from_format("{ %s %s %s }",
			get_operation_as_string(sentence->operation_id), sentence->key, sentence->value);
}

char* planifier_sentence_to_string(t_planifier_sentence* sentence) {
	if (sentence->esi_id != 0) {
		return string_from_format("{ %s %s }",
								  get_operation_as_string(sentence->operation_id), sentence->resource);
	}
	return string_from_format("{ %s %s %ld }",
							  get_operation_as_string(sentence->operation_id), sentence->resource, sentence->esi_id);
}
