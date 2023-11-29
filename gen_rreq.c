/*	
 *	FILE: $RCSFile$	
 *
 ********************************
 *
 *	General description:
 *        Generates and sends a route request mesages from the given in 
 *        parameters
 *
 *	Internal procedures:
 *	
 *	External procedures:
 *        gen_rreq
 *
 ********************************
 *
 * Extendend RCS Info: $Id: gen_rreq.c,v 1.11 2000/04/18 07:54:15 root Exp root $
 *
 */
#include "gen_rreq.h"

/* 
 *   gen_rreq
 *
 *   Description: 
 *     Generates and sends a route request message
 *     with the data in the info struct.
 *     The rreq is allways sent to the broadcast(255.255.255.255)
 *     address.
 *     The rreq is then enqueued on the prioqueue for retransmission
 *     if a RREP is not recived within the timeout period.
 *
 *   Arguments:
 *     struct info *is - The data about the addresses.
 *     is->ip_pkt_dst_ip = The destination addres in the RREQ
 *     is->ip_pkt_src_ip = The source addres in the RREQ
 *     is->ip_pkt_my_ip = Used as the hosts own address.
 *     is->ip_pkt_ttl = Not used at all.
 *
 *   Return:
 *     int - On error -1 is returned otherwise 0
*/
int
gen_rreq(struct info *is)
{
  struct artentry *rtent;
  struct rreq_tdata *trd;
  struct rreq *out_rreq;
  struct info bcast;
  
  /* Do not send rreq to the same host again if its already in the 
     timer queue */
  if (pq_getfirstofidflags(is->ip_pkt_dst_ip,PQ_PACKET_RREQ) != NULL)
    return 0;
  
  /* Allocate memory for the rreq message */
  if ((out_rreq = (struct rreq*) malloc(sizeof(struct rreq))) == NULL)
    {
      fprintf(stderr,"%s : %d : Can't allocate new rreq\n",
	      __FILE__, __LINE__);
      return -1;
    }

  /* Allocate memory for the timer queue position for the rreq */
  if ((trd = malloc(sizeof(struct rreq_tdata))) < 0)
    /* Couldn't allocate memory */
    return -1;

  /* Get routing table entry for destination */
  rtent = getentry(is->ip_pkt_dst_ip);
  
  if (rtent == NULL)
    {
      /* Entry does not exist -> set to initial values*/    
      out_rreq->broadcast_id = 0;
      out_rreq->dst_seq = 0;
      bcast.ip_pkt_ttl =  TTL_START;
    }
  else 
    {
      /* Entry does exist -> get value from rt */    
      out_rreq->broadcast_id = (rtent->broadcast_id)++;
      out_rreq->dst_seq = rtent->dst_seq;
      bcast.ip_pkt_ttl =  rtent->hop_cnt + TTL_INCREMENT;
    }
  
  
  /* Get routing table entry for source, when this is ourself this one
     should allways exist*/
  rtent = getentry(is->ip_pkt_src_ip);
  
  if (rtent == NULL)
    {
      fprintf(stderr,"%s : %d : Can't get route to self\n",
	      __FILE__, __LINE__);
      return -1;
    } 
  else 
    /* Get our own sequence number */
    out_rreq->src_seq = rtent->dst_seq;
  
  /* Fill in the package */
  out_rreq->dst_ip = is->ip_pkt_dst_ip;
  out_rreq->src_ip = is->ip_pkt_src_ip;
  out_rreq->type = 1;
  out_rreq->hop_cnt = 0;
  out_rreq->j = 0;
  out_rreq->r = 0;
  out_rreq->reserved = 0;
  
  /* Get the broadcast address and ttl right */
  bcast.ip_pkt_dst_ip = inet_addr("255.255.255.255");
  bcast.ip_pkt_my_ip = is->ip_pkt_my_ip;
  bcast.ip_pkt_src_ip = is->ip_pkt_my_ip;
  
  if (add_rreq(out_rreq->src_ip, 
	       out_rreq->broadcast_id, getcurrtime() + BCAST_ID_SAVE) == -1)
    {
      fprintf(stderr,"%s : %d : Can't add to broadcast list\n",
	      __FILE__, __LINE__);
      return -1;
    }
  
  /* Send the package */
  if (send_datagram(&bcast,out_rreq,sizeof(struct rreq)) < 0)
    {
      fprintf(stderr,"%s : %d : Can't send broadcast\n",
	      __FILE__, __LINE__);
      return -1;
    }
  
  /* Enqueue in the message queue */
  trd->rd = out_rreq;
  trd->retries = 0;
  trd->ttl = bcast.ip_pkt_ttl;
  trd->dst_ip = bcast.ip_pkt_dst_ip;
  
  /* Is this HELLO message ? */
  if (out_rreq->dst_ip == inet_addr("255.255.255.255"))
      pq_insert(getcurrtime() + HELLO_INTERVAL, trd, out_rreq->dst_ip,
		PQ_PACKET_HELLO);

  else
      pq_insert(getcurrtime() + 2 * bcast.ip_pkt_ttl * NODE_TRAVERSAL_TIME,
		trd, out_rreq->dst_ip, PQ_PACKET_RREQ);

  return 0;
}



