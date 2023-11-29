/*      
 *      FILE: $RCSFile$ 
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
 *      General description:
 *          Generates a Route Reply
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          gen_rrep(struct info, struct rreq)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: gen_rrep.c,v 1.30 2000/05/16 07:31:43 root Exp root $
 *
 */

#include "gen_rrep.h"

extern struct artentry *g_my_entry;

/*
 * gen_rrep
 *
 * Description:  
 *   Generates a RREP
 *
 * Arguments:    
 *   struct info *my_info - The information on IP level
 *   struct rreq *my_rreq - The RREQ that needs a RREP
 *
 * Return:       
 *   int - 0 (- if succesful)
 */
int
gen_rrep(struct info *my_info, struct rreq *my_rreq)
{
  struct rrep      my_rrep;
  struct artentry  *rt;
  struct artentry  *rt_src;
  u_int64_t        curr_time;  /* Current time */
 
  /* Get the source and destination IP address from the RREQ */
  my_rrep.src_ip = my_rreq->src_ip;
  my_rrep.dst_ip = my_rreq->dst_ip;
  
  /* Get the routing table entry corresponding to the asked destination */
  rt = getentry(my_rrep.dst_ip); 
  
  /* Check if the destination IP of RREQ was this node */
  if (my_rreq->dst_ip == my_info->ip_pkt_my_ip)    
    {
      /* The RREQ was for this node */

      /* Increase my source sequence number */
      (g_my_entry->dst_seq)++;

      /* Set the reply structure */
      if (my_rreq->dst_seq > rt->dst_seq) 
	rt->dst_seq = my_rreq->dst_seq;

      my_rrep.dst_seq = rt->dst_seq;
      my_rrep.hop_cnt = 0;
      my_rrep.lifetime = MY_ROUTE_TIMEOUT;
    }
  else 
    {
      /* The RREQ was for a node in RT */

      /* Set the reply structure */
      my_rrep.dst_seq = rt->dst_seq;
      my_rrep.hop_cnt = rt->hop_cnt;
      curr_time = getcurrtime(); /* Get current time */
      my_rrep.lifetime = rt->lifetime - curr_time;
      rt_src = getentry(my_rrep.src_ip); 
      
      /* Add to precursors... */
      if (add_precursor(rt_src, rt->nxt_hop) == -1)
	{
	  /* Couldn't add precursor. Ignore and continue. */
	}
    }

  /* Add to precursors... */
  if (add_precursor(rt, my_info->ip_pkt_src_ip) == -1)
    {
      /* Couldn't add precursor. Ignore and continue. */
    }

  /* Set the rest of the RREP structure */
  my_rrep.type = 2;
  my_rrep.r = 0;
  my_rrep.prefix_sz = 0;
  my_info->ip_pkt_ttl = 1;
  my_info->ip_pkt_dst_ip = my_info->ip_pkt_src_ip;
  my_info->ip_pkt_src_ip = my_info->ip_pkt_my_ip;

  if (send_datagram(my_info, &my_rrep, sizeof(my_rrep)) == -1)
    {
      /* Couldn't send the RREP. Ignore and let the sourse request again */
    }

  if (my_rreq->dst_ip != my_info->ip_pkt_my_ip)
    {
      /* Now send a datagram to the requested host telling it it has been 
	 asked for */
      
      my_rrep.type = 2;
      my_rrep.r = 0;
      my_rrep.prefix_sz = 0;
      my_info->ip_pkt_ttl = 1;
      
      /* Set the source to be the destination of the rreq (it has asked for
	 it itself)*/
      my_rrep.src_ip = my_rreq->dst_ip;
      
      /* Get info on the source */
      rt = getentry(my_rreq->src_ip); 
      
      /* Insert the rreq's source attributes */
      my_rrep.dst_ip = rt->dst_ip;
      my_rrep.dst_seq = rt->dst_seq;
      my_rrep.hop_cnt = rt->hop_cnt;
      curr_time = getcurrtime(); /* Get current time */
      my_rrep.lifetime = rt->lifetime - curr_time;
      
      /* Get info on the destination */
      rt = getentry(my_rreq->dst_ip); 
      
      my_info->ip_pkt_dst_ip = rt->nxt_hop;
      my_info->ip_pkt_src_ip = my_info->ip_pkt_my_ip;
      
      
      if (send_datagram(my_info, &my_rrep, sizeof(my_rrep)) == -1)
	{
	  /* Couldn't send the RREP. Ignore, let the sourse request again */
	}
    }
  
  return 0;
}
