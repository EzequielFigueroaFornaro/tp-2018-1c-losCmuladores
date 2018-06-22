#ifndef PLANIFIER_STRUCTURES_H_
#define PLANIFIER_STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "semaphores.h"

#include "logging.h"

typedef struct {
	long id;
	int estado;
	long tiempo_de_entrada;
	long cantidad_de_instrucciones;
	int instrucction_pointer;
	pthread_t esi_thread;
	int socket_id;
} esi;

enum tipo_de_esi {
	ESI_BLOQUEADO = -10

};

enum estados {
	NUEVO = 1,
	BLOQUEADO = 2,
	DESBLOQUEADO = 3,
	CORRIENDO = 4,
	FINALIZADO = 5
};

extern long RUNNING_ESI;
extern long NEXT_RUNNING_ESI;

t_list* READY_ESI_LIST;
t_list* BLOCKED_ESI_LIST;
t_queue* FINISHED_ESI_LIST;

t_dictionary * esis_bloqueados_por_recurso;
t_dictionary * recurso_tomado_por_esi;

long id_as_long(char* esi_id);

void queue_push_id(t_queue* queue, long id);

void list_add_id(t_list* list, long id);

void make_wait_for_resource(long esi_id, char* resource);

char* get_all_waiting_for_resource_as_string(char* resource);

#endif

