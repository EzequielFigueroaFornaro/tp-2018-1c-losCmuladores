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
message_type ISE_STOP = 300;
message_type ISE_EXECUTE = 301;
message_type ISE_KILL = 302;

t_sentence* sentence_create() {
	t_sentence* sentence = malloc(sizeof(t_sentence));
	sentence->operation_id = 0;
	sentence->key = NULL;
	sentence->value = NULL;
	return sentence;
}

void sentence_destroy(t_sentence* sentence) {
	if (NULL == sentence) {
		free(sentence->key);
		free(sentence->value);
		free(sentence);
	}
}

t_buffer serialize_sentence(t_sentence* sentence){
	int operation_length = sizeof(sentence -> operation_id);
	int key_length = strlen(sentence -> key) + 1;
	int value_length = strlen(sentence -> value) + 1;

	// operation_id + key_int + key_length + value_int + value_length
	int message_size = sizeof(int) + sizeof(int) + key_length + sizeof(int) + value_length;
	void* buffer = malloc(message_size);
	void* offset = buffer;
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

char* sentence_to_string(t_sentence* sentence) {
	if (strcmp(sentence->value, "") == 0) {
		return string_from_format("{ %s %s }",
				get_operation_as_string(sentence->operation_id), sentence->key);
	}
	return string_from_format("{ %s %s %s }",
			get_operation_as_string(sentence->operation_id), sentence->key, sentence->value);
}
