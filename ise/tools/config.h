#ifndef TOOLS_CONFIG_H_
#define TOOLS_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include "logging.h"
#include "types.h"

t_dictionary* modules_config;

typedef struct {
	char* ip;
	int port;
	int socket;
} t_module_config;

char* get_ip(module_type module);

int get_port(module_type module);

int get_socket(module_type module);

t_module_config* get_config(module_type module);

void load_config(char* config_file_path);

void destroy_config();

void set_socket(module_type module, int socket);


#endif /* TOOLS_CONFIG_H_ */
