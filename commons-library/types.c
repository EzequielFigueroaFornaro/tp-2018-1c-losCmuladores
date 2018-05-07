/*
 * types.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#include "types.h"

t_buffer* serialize_sentence(t_sentence* sentence){
	int key_length = strlen(sentence -> key) + 1;
	int value_length = strlen(sentence -> value) + 1;

	t_sentence_header* sentence_header = (t_sentence_header*) malloc(sizeof(t_sentence_header));
	sentence_header -> operation_id = sentence -> operation_id;
	sentence_header -> key_length = key_length;
	sentence_header -> value_length = value_length;

	int message_size = sizeof(t_sentence_header) + key_length + value_length;
	void* buffer = malloc(message_size);
	int offset = 0;
	memcpy(buffer, sentence_header, sizeof(t_sentence_header));
	offset += sizeof(t_sentence_header);
	memcpy(buffer + offset, (sentence -> key), key_length);
	offset += key_length;
	memcpy(buffer + offset, (sentence -> value), value_length);
	offset += value_length;

	t_buffer* buffer_struct = (t_buffer*) malloc(sizeof(t_buffer));
	buffer_struct -> buffer_content = buffer;
	buffer_struct -> size = message_size;

	free(sentence_header);

	return buffer_struct;
}

void destroy_buffer(t_buffer* buffer){
	free(buffer -> buffer_content);
	free(buffer);
}
