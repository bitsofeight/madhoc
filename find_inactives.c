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
 *        Detects routes that have expired and marks them
 *        as expired in the routing table. When an expired route is not 
 *        renewed and times out the route is deleted from the routing 
 *        table. find_inactives is called every HELLO_INTERVAL. If the 
 *        expired node is a neighbour link_break is called to inform other
 *        neighbours.
 *
 *      Internal procedures: 
 *      
 *      External procedures: 
 *        find_inactives()
 *
 ********************************
 *
 *      Extendend RCS Info: $Id$
 *
 */

#include "find_inactives.h"

extern u_int32_t         g_my_ip;
extern struct artentry  *g_my_entry;

/*
 * find_inactives
 *
 * Descritpion:
 *   Finds inactive entries 
 *
 * Arguments: Void
 *
 * Return: Void
 */
void
find_inactives()
{
  struct rt_entry_list *tmp_rt_entry_list;
  struct artentry *tmp_artentry;
  struct info tmp_info;
  
  for(tmp_rt_entry_list = get_first_entry();
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)
    {
      tmp_artentry = tmp_rt_entry_list->entry;
      if(tmp_artentry->lifetime < getcurrtime())
	{
	  if(tmp_artentry->hop_cnt == 255) /* thus time to be deleted
					      note that lifetime now after 
					      route_expiry really is time 
					      to deletion */
	    {
	      /* step back before deletion */
	      tmp_rt_entry_list = tmp_rt_entry_list->prev; 
	      delete_entry(tmp_artentry->dst_ip);
	    }
	  else /* time to be expired */
	    {
	      if(tmp_artentry->nxt_hop == tmp_artentry->dst_ip) 
		{		
		  /* thus neighbour */
		  g_my_entry->dst_seq++;
		  tmp_info.ip_pkt_my_ip = g_my_ip;
		  /* link break also performs route_expiry */
		  link_break(&tmp_info, tmp_artentry->dst_ip);
		}
	      else
		{
		  route_expiry(tmp_artentry);
		}
	    }
	}
    }
}
