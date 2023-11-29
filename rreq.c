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
 *        Receive Route Request.
 *        Receives the route requests from other nodes and
 *        acts dependingly of the route table. If the request
 *        is for it self, or it has an active route, it sends
 *        back a reply. Otherwise it forwards to requst to a
 *        new broadcast. 
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *        rec_rreq(struct info, struct rreq)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: rreq.c,v 1.27 2016/04/18 15:09:36 root Exp root $
 *
 */

#include "rreq.h"

/*
 * rec_rreq
 *
 * Description:  
 *   Receive the route request. Looks up the routing table
 *   to determine if there is an active route to the 
 *   destination. An active route is either to the node
 *   it self, or a destination that there already exists
 *   a route that isn't too old.
 *   It then calls the right functions, either a route reply
 *   or retransmits the route request in broadcast.
 *
 * Arguments:    
 *   struct info *inf - The information structure containing information on
 *                      IP-level such as IP-addresses.
 *   struct rreq *in_rreq - Structure that contains the informations about
 *                           the route request.
 *
 * Return: 
 *   int - 0 - if succesful
 *        -1 - if memory couldn't be allocated
 */
int
rec_rreq(struct info *inf, struct rreq *in_rreq)
{
  struct rreq       *out_rreq;  /* Forwarded RREQ */
  struct artentry   *rte;       /* Routing table entry */
  struct rreq_entry *tmp_rreq_entry; /* Routing request list entry */
  struct info       *out_info;  /* Outgoing information struct */
  u_int32_t          source_ip = in_rreq->src_ip;  /* RREQ sender IP */
  u_int32_t          dest_ip = in_rreq->dst_ip;    /* RREQ destination IP */
  u_int32_t          broadc_id = in_rreq->broadcast_id; /* Broadcast id */
  u_int64_t          current_time;  /* Current time */
  
  /* Update the hop count */
  in_rreq->hop_cnt++;

  /* This code takes care of HELLO messages.
   * If the destination ip in the RREQ is broadcast, change it to my ip 
   * and reply */

  if (dest_ip == inet_addr("255.255.255.255"))
    {
      in_rreq->dst_ip = inf->ip_pkt_my_ip;
      update_reverse(inf, in_rreq);
      
      gen_rrep(inf, in_rreq);
      return(0);
    }
  
  /* Look in the route request list to see if the node has 
     already received this request. */
  tmp_rreq_entry = find_rreq(source_ip, broadc_id);
  
  current_time = getcurrtime(); /* Get the current time */
  
  /* Check if an entry in the route request list was found, and if it's
     still valid. If there is a valid entry, this request shouldn't be
     checked. */
  if (!((tmp_rreq_entry != NULL) && 
	(tmp_rreq_entry->lifetime > current_time)))
    { 
      /* Have not received this RREQ within BCAST_ID_SAVE time */

      /* Add this RREQ to the list for further checks */
      if (add_rreq(source_ip, broadc_id, current_time + BCAST_ID_SAVE) == -1)
	{	
	  /* Couldn't add the entry, ignore and continue */
	}
      
      /* Look up in the routing table if there already is a route 
         to this destination */
      rte = getentry(dest_ip);
      
      /* UPDATE REVERSE */
      update_reverse(inf, in_rreq);
      
      /* Allocate memory for the info struct to the outgoing package */
      if ((out_info = (struct info*) malloc(sizeof (struct info))) == NULL)
	/* Failed to allocate memory for out_info. */
	return -1;
      
      /* Set the informtion struct */
      out_info->ip_pkt_my_ip = inf->ip_pkt_my_ip;
      out_info->ip_pkt_src_ip = inf->ip_pkt_my_ip;
      out_info->ip_pkt_dst_ip = inf->ip_pkt_src_ip;
      out_info->ip_pkt_ttl = inf->ip_pkt_ttl;
      
      /* Check if the request was for this node */
      if (dest_ip == inf->ip_pkt_my_ip) 
	{ 
	  /* The RREQ was destined to this node */
	  /* Call for gen_rrep to send a Route Reply */
	  gen_rrep(inf, in_rreq);
	  
	  free(out_info);
	  return 0;
	}
      else
	{ 
	  /* RREQ is for another node */	  
	  
	  /* Check if this node already have a route to the destination
             and if it's not too old. If those two are true, check if 
             the squence number is newer than the one in the table */ 
	  /* !!!!!! not too old == hop_count != infinity !!!!!!!!  */

	  if ((rte != NULL && rte->hop_cnt != 255) &&
	      (in_rreq->dst_seq <= rte->dst_seq))
	    {
	      /* The node already had a valid route to the destination */

	      /* Call for gen_rrep to send a Route Reply */
	      gen_rrep(inf, in_rreq);

	      free(out_info);
	      return 0;
	    }
	  else
	    {
	      /* The node didn't have a valid route to the destination */

	      /* Decreas the ttl and check if it reaches 0.
	         If so, throw the request */
	      if ((out_info->ip_pkt_ttl = inf->ip_pkt_ttl - 1) == 0)
		{
		  free(out_info);
		  
		  return 0;
		}
	      
	      /* Allocate memory for a new RREQ */
	      if ((out_rreq = (struct rreq*) 
		   malloc(sizeof (struct rreq))) == NULL)
		/* Failed to allocate memory for forwarded RREQ */
		return -1;
	      
	      /* Set the RREQ structure */
	      out_rreq->type = 1;
	      out_rreq->j = 0;
	      out_rreq->r = 0;
	      out_rreq->reserved = 0;
	      out_rreq->hop_cnt = in_rreq->hop_cnt; /* Increased hopcount */
	      out_rreq->broadcast_id = broadc_id;
	      out_rreq->dst_ip = dest_ip;
	      /* Set the right sequence number */
	      if (rte)
		out_rreq->dst_seq = MAX (in_rreq->dst_seq, rte->dst_seq);
	      else
		out_rreq->dst_seq = in_rreq->dst_seq;

	      out_rreq->src_ip = source_ip;
	      out_rreq->src_seq = in_rreq->src_seq;
	      
	      /* Set the destination IP to broadcast */
	      out_info->ip_pkt_dst_ip = inet_addr("255.255.255.255");
	      
	      /* Call send_datagram to send and forward the RREQ */
	      send_datagram(out_info, out_rreq, sizeof(struct rreq)); 

	      free(out_rreq);
	      free(out_info);
  
	      return 0;
	    }
	}
    }
  
  return 0;
}
