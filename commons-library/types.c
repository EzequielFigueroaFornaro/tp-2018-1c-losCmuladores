/*
 * types.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#include "types.h"

t_buffer serialize_sentence(t_sentence* sentence){
	int operation_length = sizeof(sentence -> operation_id);
	int key_length = strlen(sentence -> key) + 1;
	int value_length = strlen(sentence -> value) + 1;

	// operation_id + key_int + key_length + value_int + value_length
	int message_size = sizeof(int) + sizeof(int) + key_length + sizeof(int) + value_length;
	void* buffer = malloc(message_size);
	void* offset = buffer;
	offset = concat_value(offset, &sentence -> operation_id, operation_length);
	offset = concat_string(offset, sentence -> key, key_length);
	offset = concat_string(offset, sentence -> value, value_length);

	t_buffer buffer_struct;
	buffer_struct.buffer_content = buffer;
	buffer_struct.size = message_size;

	free(sentence_header);

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

void* concat_value(void* mem_address, void* value, int size_of_value) {
	memcpy(mem_address, value, size_of_value);
	return mem_address + size_of_value;
}

void* concat_string(void* mem_address, void* string, int string_length) {
	void* offset = mem_address;
	offset = concat_value(offset, &string_length, sizeof(string_length));
	return concat_value(offset, string, string_length);
}
