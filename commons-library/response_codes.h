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
	START_COMPACTION = 9,
	CALCULATE_INSTANCE_OK = 10,
	CALCULATE_INSTANCE_ERROR = 11,
	KEY_FOUND = 12
} execution_result;

char* get_execution_result_description(int execution_result);

#endif /* RESPONSE_CODES_H_ */
