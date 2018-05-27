#include "script_handler.h"

int load_script_from_file(char * file_name) {
	FILE * script_file = fopen(file_name, "r");

	if (script_file == NULL) {
		return 1;
	}

	script = malloc(sizeof(t_ise_script));
	script->lines = queue_create();
	while (!(feof(script_file))) {
		size_t line_size = 0;
		char * line;
		bool valid_line = getline(&line, &line_size, script_file) != -1;
		if (valid_line) {
			queue_push(script->lines, line);
		}
	}
	fclose(script_file);
	return 0;
}

t_ise_sentence next_sentence() {
	char* line = queue_pop(script->lines);
	t_ise_sentence sentence;
	if (line != NULL) {
		string_trim(&line);
		sentence.operation = parse(line);
		sentence.empty = false;
	} else {
		sentence.empty = true;
	}
	return sentence;
}

void destroy_script(t_ise_script * script) {
//	queue_destroy(script->lines);
	free(script);
}

void print_script(t_ise_script * script) {
	for (int i = 0; i < queue_size(script->lines); i++) {
		char* instruction = list_get(script->lines->elements, i);
		printf("%s", instruction);
	}
}


