/*	
 *	FILE: $RCSFile$	
 *
 *
 * Mad-hoc by
 *
 * Fredrik Lilieblad
 * Oskar Mattsson
 * Petra Nylund
 * Dan Ouchterlony
 * Anders Roxenhag
 *
 * Released 2000-05-27 
 * This software is Open Source under the GNU General Public Licence. 
 * 
 * Mail : mad-hoc@flyinglinux.net
 * WWW  : mad-hoc.flyinglinix.net
 *
 *
 ********************************
 *
 *	General description:
 *        Resends a queued RREQ if retries is lower than RREQ_RETRIES
 *	  otherwise it throws the package away and returns error
 *
 *	Internal procedures:
 *	
 *	External procedures:
 *        rreq_timeout
 *        hello_resend
 *
 ********************************
 *
 Extendend RCS Info: $Id: to_rreq.c,v 1.5 2000/04/18 08:12:41 root Exp root $
 *
 */
#include "to_rreq.h"


/* 
 *   rreq_timeout
 *
 *   Description: 
 *     Resends the RREQ in the input argument while keeping
 *     track of nr of times resent. If maximum times the 
 *     packet is thrown away and an error is returned by the
 *     function.
 *
 *   Arguments:
 *     struct rreq_tdata *rd - The data and the RREQ package that will be 
 *                             resent
 *
 *   Return:
 *     int - Returns -1 on error or maximum retries otherwise 0
*/
int
rreq_timeout(struct rreq_tdata *rd)
{
  struct artentry *rte;
  struct info is;
  u_int64_t currtime;

  /* Check how may time we have sent it already */
  if (rd->retries >= RREQ_RETRIES)
    {
      /* Sent it maximum times */
      
      /* Do nothing except freeing*/
      free((rd->rd));
      free(rd);
      
      /* Return error */
      return -1;
    }
  else 
    {
      /* Increment nr of retries */
      rd->retries++;
      
      /* Check new TTL */
      if (rd->ttl > TTL_TRESHOLD)
	rd->ttl = NET_DIAMETER;
      else
	rd->ttl += TTL_INCREMENT;
      
      /* Update broadcast id if available */
      if ((rte = getentry(rd->dst_ip)) != NULL)
	rte->broadcast_id++;
      
      /* Fill in struct */
      is.ip_pkt_dst_ip = rd->dst_ip;
      is.ip_pkt_ttl = rd->ttl;
      is.ip_pkt_src_ip = (rd->rd)->src_ip;
      is.ip_pkt_my_ip = (rd->rd)->src_ip;
      
      /* Update broadcast id */
      ((rd->rd)->broadcast_id)++;
      
      if (add_rreq((rd->rd)->src_ip, (rd->rd)->broadcast_id, 
		   getcurrtime() + BCAST_ID_SAVE) == -1)
	  /* Couldn't add to broadcast list */
	  return -1;
      
      /* Send packet again */
      send_datagram(&is, rd->rd, sizeof(struct rreq));
      
      /* Set a new timer */
      currtime = getcurrtime();
      pq_insert(currtime + 2 * rd->ttl * NODE_TRAVERSAL_TIME, rd, 
		(rd->rd)->dst_ip, PQ_PACKET_RREQ);
    }
  
  return 0;
}

/* 
 *   hello_resend
 *
 *   Description: 
 *     Resends the HELLO message and puts it back in the timer queue.
 *
 *   Arguments:
 *     struct rreq_tdata *rd - The data and the HELLO package that will
 *                             be resent.
 *
 *   Return: None
 */
void
hello_resend(struct rreq_tdata *rd)
{
  struct artentry *rte;
  struct info is;
  u_int64_t currtime;
  
  /* Fill in struct */
  is.ip_pkt_dst_ip = rd->dst_ip;
  is.ip_pkt_ttl = rd->ttl;
  is.ip_pkt_src_ip = (rd->rd)->src_ip;
  is.ip_pkt_my_ip = (rd->rd)->src_ip;

  if ((rte = getentry((rd->rd)->src_ip)) != NULL)
    {
      (rd->rd)->src_seq = rte->dst_seq;
      (rd->rd)->broadcast_id = rte->broadcast_id;
      (rd->rd)->hop_cnt = rte->hop_cnt;
      (rd->rd)->dst_seq = 0;
    }
  
  
  /* Send packet again */
  send_datagram(&is, rd->rd, sizeof(struct rreq));
  
  /* Set a new timer */
  currtime = getcurrtime();
  pq_insert(currtime + HELLO_INTERVAL, rd, (rd->rd)->dst_ip, 
	    PQ_PACKET_HELLO);
  
}
