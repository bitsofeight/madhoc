#ifndef RT_ENTRY_H
#define RT_ENTRY_H

#include "precursor.h"

struct artentry
{
  u_int32_t dst_ip;
  u_int32_t dst_seq;
  u_int32_t broadcast_id;
  u_int8_t hop_cnt;
  u_int8_t lst_hop_cnt;
  u_int32_t nxt_hop;   
  struct precursor *precursors; /* formerly u_int_32_t* */
  u_int64_t lifetime;
  unsigned short int rt_flags;
};

#endif

