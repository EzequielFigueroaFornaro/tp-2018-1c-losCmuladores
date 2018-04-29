/*
 * types.h
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#ifndef TYPES_H_
#define TYPES_H_

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


#endif /* TYPES_H_ */
