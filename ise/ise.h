#include <stdio.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "commons-sockets.h"

t_log * logger;
int planifier_port;
char* planifier_ip;

int coordinator_port;
char* coordinator_ip;

int handshake_planifier();

void exit_gracefully(int code);

void load_configuration(char* config_file_path);

void configure_logger();
