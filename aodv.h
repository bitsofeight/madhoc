#ifndef AODV_H
#define AODV_H

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define AODVPORT               1303

#define ACTIVE_ROUTE_TIMEOUT   9000
#define ALLOWED_HELLO_LOSS     2
#define HELLO_INTERVAL         3000
#define NODE_TRAVERSAL_TIME    100
#define NET_DIAMETER           35
#define RREP_WAIT_TIME         (3 * (NODE_TRAVERSAL_TIME) * (NET_DIAMETER))/2
#define BAD_LINK_LIFEIME       2 * (RREP_WAIT_TIME)
#define BCAST_ID_SAVE          30000
#define BROADCAST_RECORD_TIME  (RREP_WAIT_TIME)
#define DELETE_PERIOD          5 * MAX((ACTIVE_ROUTE_TIMEOUT),((ALLOWED_HELLO_LOSS)*(HELLO_INTERVAL)))
#define GROUP_HELLO_INTERVAL   5000
#define REV_ROUTE_LIFE         (RREP_WAIT_TIME)
#define MTREE_BUILD            2 * (REV_ROUTE_LIFE)
#define MY_ROUTE_TIMEOUT       2 * (ACTIVE_ROUTE_TIMEOUT)
#define NEXT_HOP_WAIT          (NODE_TRAVERSAL_TIME) + 10
#define PRUNE_TIMEOUT          (ACTIVE_ROUTE_TIMEOUT)
#define RREQ_RETRIES           2
#define TTL_START              1
#define TTL_INCREMENT          2
#define TTL_TRESHOLD           7
#define RREQ                   1
#define RREP                   2
#define RERR                   3

#define SECOND                 1000

struct rreq
{
  u_int8_t     type;
  unsigned int j:1;
  unsigned int r:1;
  unsigned int reserved:14;
  u_int8_t     hop_cnt;
  u_int32_t    broadcast_id;
  u_int32_t    dst_ip;
  u_int32_t    dst_seq;
  u_int32_t    src_ip;
  u_int32_t    src_seq;
};

struct rrep
{
  u_int8_t     type;
  unsigned int r:1;
  unsigned int reserved:10;
  unsigned int prefix_sz:5;
  u_int8_t     hop_cnt;
  u_int32_t    dst_ip;
  u_int32_t    dst_seq;
  u_int32_t    src_ip;
  u_int32_t    lifetime;
};

struct rerr_unr_dst
{
  u_int32_t unr_dst_ip;
  u_int32_t unr_dst_seq;
  struct rerr_unr_dst *next;
};

struct rerrhdr
{
  unsigned int type:8;
  unsigned int reserved:16;
  unsigned int dst_cnt:8;
  struct rerr_unr_dst *unr_dst;
};

struct rerr
{
  unsigned int type:8;
  unsigned int reserved:16;
  unsigned int dst_cnt:8;
};

struct rerrdst
{
  u_int32_t unr_dst_ip;
  u_int32_t unr_dst_seq;
};

#endif






