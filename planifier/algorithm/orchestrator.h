#include "fifo.h"


char* ALGORITHM;
t_list * READY_ESI_LIST;
t_list * RUNNING_ESI_LIST;
t_list * BLOCKED_ESI_LIST;
t_list * FINISHED_ESI_LIST;

enum algorithm {
	FIFO = "FIFO"
} algorithm ;

void set_orchestrator(char* algorithm);
