#include "commons-list.h"


//TODO test de esto
void* list_filter_and_remove(t_list *list, bool(*condition)(void*)) {
      int size = list_size(list);
      t_list* sublist = list_create();
       for (int i = 0; i < size; ++i) {
    	   esi* esi = list_get(list,i);
       }
    	   if (condition(esi)) {
    		   list_add(sublist, esi);
               list_remove(list, i);
               size --;
               i--;
    	   }
       }
       return sublist;
}


