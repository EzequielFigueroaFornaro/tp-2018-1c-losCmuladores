/*
 * data.h
 *
 *  Created on: 8 jun. 2018
 */

#ifndef DATA_H_
#define DATA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>

t_dictionary * esis_bloqueados_por_recurso;
t_dictionary * recurso_tomado_por_esi;

extern pthread_mutex_t map_boqueados;

#endif /* DATA_H_ */
