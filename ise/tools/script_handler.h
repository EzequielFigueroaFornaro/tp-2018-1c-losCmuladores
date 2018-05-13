#ifndef SCRIPT_HANDLER_H_
#define SCRIPT_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <commons/collections/queue.h>
#include <parsi/parser.h>

typedef struct {
	t_queue * lines;
} t_ise_script;

t_ise_script * script;

typedef struct {
	t_esi_operacion operation;
	bool empty;
} t_ise_sentence;


int load_script_from_file(char * file_name);

t_ise_sentence next_sentence();

void destroy_script(t_ise_script * script);

void print_script(t_ise_script * script);


#endif /* SCRIPT_HANDLER_H_ */
