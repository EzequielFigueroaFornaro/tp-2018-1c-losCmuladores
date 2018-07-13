#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Dispatcher
extern pthread_mutex_t notify_dispatcher_mtx;
extern pthread_mutex_t dispatcher_manager;
extern pthread_cond_t resume_planification;
extern pthread_cond_t paused_planification;
extern pthread_mutex_t pause_manager;
extern pthread_cond_t permission_to_block;
extern pthread_cond_t permission_to_block_released;
extern pthread_mutex_t permission_to_block_manager;

extern pthread_mutex_t esi_map_mtx_6;
extern pthread_mutex_t running_esi_mtx_1;
extern pthread_mutex_t next_running_esi_mtx_2;
extern pthread_mutex_t ready_list_mtx_4;
extern pthread_mutex_t blocked_list_mtx_3;
extern pthread_mutex_t finished_list_mtx_5;

extern pthread_mutex_t blocked_by_resource_map_mtx;
extern pthread_mutex_t blocked_resources_map_mtx;

extern pthread_mutex_t id_mtx;
extern pthread_mutex_t cpu_time_mtx;
