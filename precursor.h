#ifndef PRECURSOR_H
#define PRECURSOR_H

struct precursor
{
  u_int32_t ip;
  struct precursor* next;
  struct precursor* prev;
  int ishead;
};

#endif
