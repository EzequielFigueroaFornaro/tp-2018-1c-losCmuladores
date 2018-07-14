#ifndef PLANIFIER_STRUCTURES_H_
#define PLANIFIER_STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/string.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "semaphores.h"
#include "logging.h"

typedef struct {
	long id;
	int estado;
	long tiempo_de_entrada;
	long ultima_entrada_a_ready;
	int cantidad_de_instrucciones;
	int instruction_pointer;
	char* blocking_resource;
	pthread_t esi_thread;
	int socket_id;
	int duracion_real_ultima_rafaga;
	float estimacion_ultima_rafaga;
} esi;

enum tipo_de_esi {
	ESI_BLOQUEADO = -10

};

typedef enum {
	NUEVO = 1,
	BLOQUEADO = 2,
	CORRIENDO = 3,
	FINALIZADO = 4,
	DESBLOQUEADO=5
} estado;

t_dictionary * esi_map;

extern long RUNNING_ESI;
extern long NEXT_RUNNING_ESI;

t_list* READY_ESI_LIST;
t_list* BLOCKED_ESI_LIST;
t_queue* FINISHED_ESI_LIST;

t_dictionary * esis_bloqueados_por_recurso;
t_dictionary * recurso_tomado_por_esi;

long id_as_long(char* id);

char* id_to_string(long id);

char* list_join(t_list* list);

void queue_push_id(t_queue* queue, long id);

void list_add_id(t_list* list, long id);

void dictionary_put_id(t_dictionary* map, char* key, long id);

char* esis_to_string();

char* esi_to_string(esi* esi);

void list_remove_esi(t_list* list, long esi_id);

esi* get_esi_by_id(long esi_id);

bool string_is_blank(char* string);

char* esi_status_to_string(estado status);

bool is_resource_taken_by_esi(long esi_id, char* resource);

t_list* get_resources_taken_by_esi(long esi_id);

#endif

