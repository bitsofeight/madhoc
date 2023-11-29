#ifndef RT_ENTRY_LIST_H
#define RT_ENTRY_LIST_H

#include "rt_entry.h"

struct rt_entry_list
{
  struct artentry* entry;
  struct rt_entry_list* next;
  struct rt_entry_list* prev;
  int ishead;
};


#endif

