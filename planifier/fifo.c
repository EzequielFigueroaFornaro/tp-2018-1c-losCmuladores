#include "fifo.h"


esi* fifo_desbloquea_esis(int esis_id_liberadas){
//	t_list * esi_liberadas = list_filter_and_remove(blocked_esi_list,(void*)es_un_esi_libre);
//	list_add_all(ready_esi_list, esi_liberadas);
return NULL;
} // sacar de la lista de bloqueados y poner en rdy

void fifo_bloquea_esi(esi* esi){
//	remove_from_list(running_esi_list, 0, running_esi_sem_list, esi);
//	put_on_list(blocked_esi_list, esi, blocked_esi_sem_list);
	//mandar_a_bloquear(esi)
} // sacar de lista poner en lista de bloqueados

void fifo_add_esi(esi* esi){}

void fifo_finish_esi(esi* esi){}

void fifo_add_block_esi(esi* esi){}


void put_on_list(t_list* list ,esi* esi, pthread_mutex_t sem_list){
	pthread_mutex_lock(&sem_list);
	list_add(list, esi);
	pthread_mutex_unlock(&sem_list);
}

void remove_from_list(t_list* list, int index, pthread_mutex_t sem_list, esi* esi){
	pthread_mutex_lock(&sem_list);
	esi = list_remove(list, index);
	pthread_mutex_unlock(&sem_list);
}

void get_more_priority_esi(t_list * list, esi* esi){
	remove_from_list(list, 0, ready_esi_sem_list, esi);
}

void ejecutar_esi(esi* esi){
//	get_more_priority_esi(ready_esi_list, esi);
//	put_on_list(running_esi_list, esi, &running_esi_sem_list);
//	mandar_a_correr(esi);
} //mandar mensaje de que se ejecute y poner en lista de ejecutados

