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
 *          Updates the routing table when receiving RREQ
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          update_reverse(struct info *my_info, struct rreq *my_rreq)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: update_reverse.c,v 1.3 2000/04/18 09:02:43 root Exp root $
 *
 */

#include "update_reverse.h"

/* 
 *  update_reverse
 * 
 *  Description:  
 *    Updates the RT
 *
 *  Arguments:    
 *    struct info *my_info - Contains the IP informtion about the RREQ
 *    struct rreq *my_rreq -  Contains the RRQ
 *
 *  Return:
 *    int - 0
*/
int
update_reverse(struct info *my_info, struct rreq *my_rreq)
{
  struct artentry  *rt_src;
  u_int64_t        curr_time;
  
  curr_time = getcurrtime(); /* Get current time */
  rt_src = getentry(my_rreq->src_ip); /* Get entry from RT if there is one */
  
  if ((rt_src == NULL) || (my_rreq->src_seq > rt_src->dst_seq) ||
      ((my_rreq->src_seq == rt_src->dst_seq) &&
       (my_rreq->hop_cnt < rt_src->hop_cnt)))
    {
      
      /* If there didn't exist an entry in RT to the source, create it */
      if (rt_src == NULL)
	{
	  rt_src = insert_entry();
	  rt_src->dst_ip = my_rreq->src_ip;
	  rt_src->broadcast_id = 0;
	  rt_src->lst_hop_cnt = 0;
	  rt_src->lifetime = 0;
	}
      
      else /*Since the entry existed we might want to change krt*/
	del_kroute(rt_src->dst_ip, rt_src->nxt_hop);

      /* Update values in the RT entry */
      rt_src->dst_seq = my_rreq->src_seq;
      rt_src->nxt_hop = my_info->ip_pkt_src_ip;
      rt_src->hop_cnt = my_rreq->hop_cnt;
      if(add_kroute(rt_src->dst_ip, rt_src->nxt_hop))
	{
	  /* add_kroute failed, ignore and continue */
	}
    }
  
  /* Check if the lifetime in RT is valid, if not update it */
  if (rt_src->lifetime < (REV_ROUTE_LIFE + curr_time))
    rt_src->lifetime = REV_ROUTE_LIFE + curr_time;
  
  return 0;
}
