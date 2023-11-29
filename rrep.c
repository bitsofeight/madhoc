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
 *          Receives Route Reply
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          rec_rrep(struct info, struct rrep)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: rrep.c,v 1.20 2000/05/10 18:34:11 root Exp root $
 *
 */
#include "rrep.h"

/*
 * rec_rrep
 *
 * Description:  
 *   Receives and handles RREP
 *
 * Arguments:    
 *   struct info *my_info - Pointer to the received information structure
 *   struct rrep *my_rrep - Pointer to the received route reply structure
 *
 * Return:
 *   int - 0 
 */
int
rec_rrep(struct info *my_info, struct rrep *my_rrep)
{
  u_int64_t        curr_time;
  struct artentry  *rt_src; 
  struct artentry  *rt;
  
  /* Remove RREQ from resend-queue */
  pq_deleteidflags(my_rrep->dst_ip, PQ_PACKET_RREQ);

  /* Get entry from Routing Table */
  rt = getentry(my_rrep->dst_ip); 

  /* Check if there were an entry in the RT */
  if (rt != NULL)
    {
      /* Check if the entry already in RT is better than the received */
      if(rt->dst_seq >= my_rrep->dst_seq)
	{   
	  if(rt->dst_seq != my_rrep->dst_seq)
	    return 0;

	  else if(my_rrep->hop_cnt > rt->hop_cnt)
	    return 0;
	}
      del_kroute(rt->dst_ip, rt->nxt_hop);
    }
  else
    {
      /* No entry in RT found, generate a new */
      rt = insert_entry();
      rt->dst_ip = my_rrep->dst_ip;
      rt->dst_seq = 0;
      rt->broadcast_id = 0;
      rt->hop_cnt = my_rrep->hop_cnt + 1;
      rt->lst_hop_cnt = 0;
      rt->nxt_hop = my_info->ip_pkt_src_ip;
    }

  /* If the RREP is fresh I update the corresponding entry in my 
     Routing Table */  
  rt->nxt_hop = my_info->ip_pkt_src_ip;
  rt->hop_cnt = my_rrep->hop_cnt + 1;
  curr_time = getcurrtime();    /* Get current time */
  rt->lifetime = curr_time + my_rrep->lifetime;
  rt->dst_seq = my_rrep->dst_seq;
  
  if(add_kroute(rt->dst_ip, rt->nxt_hop))
    {
      /* add_kroute failed in update_reverse, ignore and continue */ 
    }
  
  if(my_rrep->src_ip != my_info->ip_pkt_my_ip) 
    /* If I'm not the destination of the RREP I forward it */
    {
      /* Update the information structure */
      my_info->ip_pkt_ttl = 1;
      my_info->ip_pkt_src_ip = my_info->ip_pkt_my_ip;
      my_rrep->hop_cnt = my_rrep->hop_cnt + 1;
      
      /* Get the entry to the source from RT */
      rt_src = getentry(my_rrep->src_ip); 

      /* Add to precursors... */
      if (add_precursor(rt, rt_src->nxt_hop) == -1)
	{
	  /* Couldn't add precursor. Ignore and continue */
	}
      rt->lifetime = curr_time + ACTIVE_ROUTE_TIMEOUT;
      my_info->ip_pkt_dst_ip = rt_src->nxt_hop;

      if (send_datagram(my_info, my_rrep, sizeof(struct rrep)) == -1)
	{
	  /* Couldn't send RREP. Ignore and let source resend RREQ */
	}
    }
  
  return 0;
}
