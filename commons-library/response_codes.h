/*
 * response_codes.h
 */

#ifndef RESPONSE_CODES_H_
#define RESPONSE_CODES_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>

typedef enum {
	OK,
	KEY_TOO_LONG,
	KEY_NOT_FOUND,
	KEY_UNREACHABLE,
	KEY_LOCK_NOT_ACQUIRED,
	KEY_BLOCKED,
	PARSE_ERROR,
	NEED_COMPACTION,
	START_COMPACTION
} execution_result;

typedef enum {
	NOT_ALLOW= 0, SUCCESS = 1
} operation_result;

char* get_execution_result_description(int execution_result);

#endif /* RESPONSE_CODES_H_ */
