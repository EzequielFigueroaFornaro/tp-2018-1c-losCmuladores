#ifndef SCRIPT_HANDLER_H_
#define SCRIPT_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <commons/collections/queue.h>
#include <parsi/parser.h>
#include "exit_handler.h"
#include "types.h"

typedef struct {
	t_queue * lines;
} t_ise_script;

t_ise_script * script;
char* script_name;

typedef struct {
	t_esi_operacion operation;
	bool empty;
} t_ise_sentence;

void load_script(char * file_name);

t_ise_sentence get_sentence_to_execute();

void destroy_script(t_ise_script * script);

void print_script(t_ise_script * script);

t_sentence* map_to_sentence(t_esi_operacion operation);

long get_script_size();

void set_retry_current_sentence(bool retry);

bool should_retry_current_sentence();

#endif /* SCRIPT_HANDLER_H_ */
