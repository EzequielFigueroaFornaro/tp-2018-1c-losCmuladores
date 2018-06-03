#ifndef COMMONS_LIST_H_
#define COMMONS_LIST_H_

#include <commons/collections/list.h>

void* list_filter_and_remove(t_list *list, bool(*condition)(void*));

#endif
