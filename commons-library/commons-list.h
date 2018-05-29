#include <commons/collections/list.h>

void* list_filter_and_remove(t_list *list, bool(*condition)(void*));
