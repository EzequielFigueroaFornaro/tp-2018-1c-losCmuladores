#ifndef TOOLS_CONFIG_H_
#define TOOLS_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include "logging.h"

t_dictionary* modules_config;

typedef struct {
	char* ip;
	int port;
	int socket;
} t_module_config;

char* get_ip(int module);

int get_port(int module);

int get_socket(int module);

t_module_config* get_config(int module);

void load_config(char* config_file_path, int coordinator, int planifier);

void destroy_config();


#endif /* TOOLS_CONFIG_H_ */
