#include "semaphores.h"

// Dispatcher
pthread_mutex_t dispatcher_manager = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resume_planification = PTHREAD_COND_INITIALIZER;
pthread_cond_t paused_planification = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pause_manager = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t permission_to_block = PTHREAD_COND_INITIALIZER;
pthread_cond_t permission_to_block_released = PTHREAD_COND_INITIALIZER;
pthread_mutex_t permission_to_block_manager = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t esi_map_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t next_running_esi_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ready_list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blocked_list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t finished_list_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t blocked_by_resource_map_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t id_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cpu_time_mtx = PTHREAD_MUTEX_INITIALIZER;
