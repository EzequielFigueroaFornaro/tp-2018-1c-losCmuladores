#include "sjf.h"

void sjf_add_esi(long esi){
	pthread_mutex_lock(&ready_list_mtx_4);
	//TODO pasa el mapa de esis, hacer la cuenta , solo hay que pasar el que esta corriendo y el nuevo, si tiene mas prioridad
	// el nuvo lo pongo a correr sino lo pongo en la lista, en un add no hace falta ver todos los que estan en la cola
	list_add_id(READY_ESI_LIST, esi);
	pthread_mutex_unlock(&ready_list_mtx_4);
}

void sjf_block_esi(long block_esi_id){
	pthread_mutex_lock(&running_esi_mtx_1);
	pthread_mutex_lock(&blocked_list_mtx_3);

	if(RUNNING_ESI == block_esi_id){
		sjf_replan();
	}else{
		bool equals_esi (long esi_id) {
				  return block_esi_id == esi_id;
		}
		pthread_mutex_lock(&ready_list_mtx_4);
		list_remove_by_condition(READY_ESI_LIST, (void*) equals_esi);
		pthread_mutex_unlock(&ready_list_mtx_4);
	}

	list_add_id(BLOCKED_ESI_LIST, block_esi_id);
	pthread_mutex_unlock(&blocked_list_mtx_3);
	pthread_mutex_unlock(&running_esi_mtx_1);
}

void sjf_finish_esi(){
	sjf_replan();
}

void sjf_replan(){
	bool shortest_job(long* esi_id, long* other_esi_id){
		esi* other_esi = (esi*) dictionary_get(esi_map, id_to_string(*esi_id));
		esi* _esi = (esi*) dictionary_get(esi_map, id_to_string(*other_esi_id));
//		long remanente_del_esi = (_esi -> cantidad_de_instrucciones) - (_esi -> cantidad_de_instrucciones);
//		long remanente_del_otro_esi = (_esi -> cantidad_de_instrucciones) - (_esi -> cantidad_de_instrucciones);
//		return (remanente_del_esi > remanente_del_otro_esi) || (remanente_del_esi == remanente_del_otro_esi && (other_esi->estado)==DESBLOQUEADO);
		int rafaga_estimada_esi = estimate_next_cpu_burst(_esi);
		int rafaga_estimada_other_esi = estimate_next_cpu_burst(other_esi);
		return (rafaga_estimada_other_esi > rafaga_estimada_esi)
				|| (rafaga_estimada_esi == rafaga_estimada_other_esi
						&& _esi->estado == DESBLOQUEADO);
	}
	pthread_mutex_lock(&next_running_esi_mtx_2);
	pthread_mutex_lock(&ready_list_mtx_4);
	pthread_mutex_lock(&esi_map_mtx_6);
	list_sort(READY_ESI_LIST, (void*) shortest_job);
	long* next_esi = list_remove(READY_ESI_LIST, 0);
	if (next_esi == NULL) {
		NEXT_RUNNING_ESI = 0;
		READY_ESI_LIST = list_create();
	} else {
		NEXT_RUNNING_ESI = *next_esi;
		log_debug(logger, "Next ESI to run is ESI%ld", NEXT_RUNNING_ESI);
	}
	pthread_mutex_unlock(&esi_map_mtx_6);
	pthread_mutex_unlock(&ready_list_mtx_4);
	pthread_mutex_unlock(&next_running_esi_mtx_2);
}
