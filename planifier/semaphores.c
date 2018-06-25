#include "semaphores.h"

pthread_mutex_t start_planification = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t esi_map_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t next_running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ready_list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blocked_list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t finished_list_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t blocked_by_resource_map_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t id_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cpu_time_mtx = PTHREAD_MUTEX_INITIALIZER;
