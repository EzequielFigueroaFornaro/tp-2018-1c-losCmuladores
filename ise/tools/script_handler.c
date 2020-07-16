#include "script_handler.h"

t_ise_sentence current_sentence;
bool retry_current_sentence = false;

char* get_file_name(char* path) {
	char** splitted_path = string_split(path, "/");
	char** ptr = splitted_path;
	char* file_name;
	for (char* c = *ptr; c; c = *++ptr) {
		file_name = c;
	}
	free(splitted_path);
	return file_name;
}

void load_script(char * file_path) {
	FILE * script_file = fopen(file_path, "r");

	if (script_file == NULL) {
		log_error(logger, "File %s not found", file_path);
		exit_with_error();
	}
	script_name = get_file_name(file_path);
	log_info(logger, "Cargando script '%s'", script_name);
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
}

t_ise_sentence get_sentence_to_execute() {
	if (!retry_current_sentence) {
		char* line = queue_pop(script->lines);
		t_ise_sentence sentence;
		if (line != NULL) {
			string_trim(&line);
			sentence.operation = parse(line);
			sentence.empty = false;
		} else {
			sentence.empty = true;
		}
		current_sentence = sentence;
		return sentence;
	}
	return current_sentence;
}

void destroy_script(t_ise_script * script) {
	if (script != NULL) {
		if (script->lines != NULL) {
			queue_destroy(script->lines);
		}
		free(script);
	}
}

void print_script(t_ise_script * script) {
	for (int i = 0; i < queue_size(script->lines); i++) {
		char* instruction = list_get(script->lines->elements, i);
		printf("%s", instruction);
	}
}

t_sentence* map_to_sentence(t_esi_operacion operation) {
	t_sentence* sentence = malloc(sizeof(t_sentence));
	switch(operation.keyword) {
	case GET:
		sentence->operation_id = GET_SENTENCE;
		sentence->key = operation.argumentos.GET.clave;
		sentence->value = "";
		break;
	case SET:
		sentence->operation_id = SET_SENTENCE;
		sentence->key = operation.argumentos.SET.clave;
		sentence->value = operation.argumentos.SET.valor;
		break;
	case STORE:
		sentence->operation_id = STORE_SENTENCE;
		sentence->key = operation.argumentos.STORE.clave;
		sentence->value = "";
		break;
	default:
		log_error(logger,"Unexpected error: operation %d not recognized", operation.keyword);
		exit_with_error();
	}
	return sentence;
}

long get_script_size() {
	return script == NULL ? 0 : (script->lines == NULL ? 0 : queue_size(script->lines));
}

void set_retry_current_sentence(bool retry) {
	retry_current_sentence = retry;
}

bool should_retry_current_sentence() {
	return retry_current_sentence;
}


