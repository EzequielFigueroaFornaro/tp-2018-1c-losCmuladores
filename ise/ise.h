#include <stdio.h>
#include <readline/readline.h> // Para usar readline
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "commons-sockets.h"

struct {
	int operation_id;
	long entries_size;
	long entries_quantity;
}__attribute((packed)) t_planifier_protocol;

t_log * logger;
int planifier_port;
char* planifier_ip;

int coordinator_port;
char* coordinator_ip;


int handshake_planifier();

void exit_gracefully(int code);

void load_configuration_2(char* config_file_path);

void configure_logger();
