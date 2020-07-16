#ifndef _ORCHESTRATOR_H_
#define _ORCHESTRATOR_H_


#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "commons/collections/dictionary.h"
#include "commons/string.h"

#include "fifo.h"
#include "sjf.h"
#include "sjf_desalojo.h"
#include "hrrn.h"

#include "planifier_structures.h"
#include "semaphores.h"
#include "logging.h"
#include "exit_handler.h"
#include "esi_connector.h"

typedef enum {
	FIFO = 1,
	SJF = 2,
	SJF_DESALOJO = 3,
	HRRN = 4
} planifier_algorithm;

planifier_algorithm algorithm;
float alpha;
float initial_estimation;

void add_esi(esi* esi);

void set_orchestrator();

bool is_valid_esi(long esi_id);

bool esi_exists(long esi_id);

long esi_se_va_a_ejecutar();

void borrado_de_finish();

void block_esi(long esi_id);

void unblock_esi(long esi_id);

void finish_esi(long esi_id);

bool bloquear_recurso(char* recurso, long esi_id);

t_queue* get_all_waiting_for_resource(char* resource);

char* get_all_waiting_for_resource_as_string(char* resource, char* separator);

void replan_by_algorithm();

long increment_id();

long cpu_time_incrementate();

long get_current_time();

void notify_dispatcher();

bool resource_taken(char* resource, long esi_id);
bool resource_taken_by_any_esi(char* resource);

void cambiar_recurso_que_lo_bloquea(char* recurso, long esi_id);

t_list* buscar_deadlock();
//TODO buscar nombre copado para la funcion
t_list* buscar_deadlock_en_lista(long id, t_list* corte);

void free_resource(char* resource);

float estimate_next_cpu_burst(esi* esi);

bool all_same_estimation();

void modificar_estado(long esi_id, estado nuevo_estado);

#endif
