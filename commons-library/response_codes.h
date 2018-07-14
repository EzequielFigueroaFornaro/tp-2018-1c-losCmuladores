/*
 * response_codes.h
 */

#ifndef RESPONSE_CODES_H_
#define RESPONSE_CODES_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>

typedef enum {
	OK = 1,
	KEY_TOO_LONG = 2,
	KEY_NOT_FOUND = 3,
	KEY_UNREACHABLE = 4,
	KEY_LOCK_NOT_ACQUIRED = 5,
	KEY_BLOCKED = 6,
	PARSE_ERROR = 7,
	NEED_COMPACTION = 8,
	INSTANCE_AVAILABLE_FOR_KEY = 10,
	NO_INSTANCE_AVAILABLE_FOR_KEY = 11,
	KEY_FOUND = 12,
	KEY_VALUE_FOUND = 13,
	KEY_VALUE_NOT_FOUND = 14
} execution_result;

char* get_execution_result_description(int execution_result);

#endif /* RESPONSE_CODES_H_ */
