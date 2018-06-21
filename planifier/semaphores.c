#include "semaphores.h"

pthread_mutex_t esi_map_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t next_running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ready_list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blocked_list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t finished_list_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t map_boqueados = PTHREAD_MUTEX_INITIALIZER;
