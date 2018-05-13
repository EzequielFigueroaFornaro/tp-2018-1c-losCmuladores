#ifndef CONNECTIONS_H_
#define CONNECTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include "logging.h"

int coordinator_port;
char* coordinator_ip;

int planifier_port;
char* planifier_ip;

bool init_connections();

#endif /* CONNECTIONS_H_ */
