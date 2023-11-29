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
 *        This module handles the handling of Route Error
 *        (RERR) messages. RERR messages are broadcasted when the node
 *        detects a broken link to one of its neighbors, it receives a
 *        packet destined for a node it does not have an active route to
 *        or when it receives a RERR message from another node. The module
 *        handles the receiption and generation of RERR messages.
 *
 *	Internal procedures: 
 *        free_rerrhdr
 *        send_rerrhdr
 *
 *	External procedures: 
 *        link_break
 *        host_unr
 *        rec_rerr
 *        create_rerrhdr
 *        append_unr_dst
 *        print_rerrhdr
 *        route_expiry
 *
 ********************************
 *
 * Extendend RCS Info: $Id: rerr.c,v 1.13 2000/04/18 17:42:04 root Exp root $
 *
 */

#include "rerr.h"

void free_rerrhdr(struct rerrhdr *tmp_rerrhdr);
int send_rerr(struct info *tmp_info, struct rerrhdr *tmp_rerrhdr);
extern u_int32_t g_my_ip;

/*
 * link_break
 *
 * Description: 
 *   link_break is called when a broken link to a neighbouring
 *   is detected. All active routes that have the unreachable node as next
 *   hop are invalidated. All precursors for this entry are removed. The
 *   RERR meassage including the unreachable destinations and their
 *   incremented seq numbers is finally rebroadcast.
 *
 * Arguments: 
 *   struct info *tmp_info - info structure
 *   u_int32_t brk_dst_ip -  IPaddress of the lost neighbour
 *
 * Returns: 
 *   int - 0 on success
 *        -1 on failure
 */
int
link_break(struct info *tmp_info, u_int32_t brk_dst_ip)
{
  struct rt_entry_list *tmp_rt_entry_list;
  struct rerrhdr *new_rerrhdr = NULL;
  struct artentry *tmp_rtentry;
  int rerrhdr_created = 0;

  for(tmp_rt_entry_list = get_first_entry();
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)
    {
      if(tmp_rt_entry_list->entry->nxt_hop == brk_dst_ip &&
	 tmp_rt_entry_list->entry->hop_cnt != 255) /* thus active */
	{
	  tmp_rtentry = tmp_rt_entry_list->entry;
	  
	  route_expiry(tmp_rtentry);
	  delete_precursor_from_all(tmp_rt_entry_list->entry->nxt_hop);
	  if(!rerrhdr_created)
	    { 
	      if((new_rerrhdr =
		  create_rerrhdr(tmp_rtentry->dst_ip,
				 tmp_rtentry->dst_seq)) == NULL)
		return -1;
	      
	      rerrhdr_created = 1;
	    }
	  else
	    {
	      if((append_unr_dst(new_rerrhdr, tmp_rtentry->dst_ip,
				 tmp_rtentry->dst_seq)) == -1)
		break; 
	    }
	  clear_precursors(tmp_rtentry);
	}
    }
  
  if(rerrhdr_created)
    {
      send_rerr(tmp_info, new_rerrhdr);
      free_rerrhdr(new_rerrhdr);
    }
  
  return 0;
}

/*
 * host_unr
 *
 * Description: 
 *   host_unr is called when a packet is received destined for a node
 *   which the forwarding node does not have an active route to. A RERR 
 *   message is created to inform neighbours.
 *
 * Arguments: 
 *   struct info *tmp_info - info structure
 *   u_int32_t brk_dst_ip - IPaddress of the unreachable destination
 *
 * Returns: 
 *   int - 0 on success
 *        -1 on failure
 */
int
host_unr(struct info *tmp_info, u_int32_t brk_dst_ip)
{
  struct rerrhdr *new_rerrhdr = NULL;
  struct artentry *tmp_rtentry;

  tmp_rtentry = getentry(brk_dst_ip);
  if(tmp_rtentry != NULL &&
     brk_dst_ip != g_my_ip)
    {
      if(tmp_rtentry->hop_cnt != 255) /* thus active route exists in RT */
	{
	  tmp_rtentry->dst_seq++;
	  tmp_rtentry->lst_hop_cnt = tmp_rtentry->hop_cnt;
	  tmp_rtentry->hop_cnt = 255;
	  if((new_rerrhdr = create_rerrhdr(brk_dst_ip, 
					   tmp_rtentry->dst_seq)) == NULL)
	    return -1;
	  
	  if(tmp_rtentry->nxt_hop == tmp_rtentry->dst_ip) 
	    /* neighbouring node */
	    delete_precursor_from_all(tmp_rtentry->nxt_hop);
	  
	  clear_precursors(tmp_rtentry);
	}
      
      send_rerr(tmp_info, new_rerrhdr);
      free_rerrhdr(new_rerrhdr);
    }
  
  else if(tmp_rtentry != NULL && brk_dst_ip == g_my_ip)
    {
      if((new_rerrhdr = create_rerrhdr(g_my_ip,
				       tmp_rtentry->dst_seq)) == NULL)
	return -1;
      
      send_rerr(tmp_info, new_rerrhdr);
      free_rerrhdr(new_rerrhdr);
    }
  
  else
    {
      if((new_rerrhdr = create_rerrhdr(brk_dst_ip, 1)) == NULL)
	return -1;
      
      send_rerr(tmp_info, new_rerrhdr);
      free_rerrhdr(new_rerrhdr);
    }
  
  return 0;
}


/*
 * rec_rerr
 *
 * Description: 
 *   rec_rerr is called when the node receives a RERR packet from
 *   another node. If the precursor list for a broken destinations is
 *   not empty a new RERR is created for that destination.
 *
 * Arguments: 
 *   struct info *tmp_info - info structure
 *   struct rrerhdr *tmp_rerrhdr - pointer to the incoming RERR message
 *
 * Returns: 
 *   int - 0 on success
 *        -1 on failure
 */
int
rec_rerr(struct info *tmp_info, struct rerrhdr *tmp_rerrhdr)
{
  struct rerrhdr *new_rerrhdr = NULL;
  struct rerr_unr_dst *tmp_unr_dst;
  struct artentry *tmp_rtentry;
  int new_rerr_created = 0;
  int i;
  
  for(i = 0, tmp_unr_dst = tmp_rerrhdr->unr_dst;
      i < tmp_rerrhdr->dst_cnt;
      i++, tmp_unr_dst = tmp_unr_dst->next)
    {
      tmp_rtentry = getentry(tmp_unr_dst->unr_dst_ip);
      /*Is the sender of the rerr the next hop for a
	broken destination for the current node? */
      if(tmp_rtentry != NULL && 
	 tmp_rtentry->dst_ip != g_my_ip && /* not the route to myself */
	 tmp_rtentry->nxt_hop == tmp_info->ip_pkt_src_ip)
	{
	  tmp_rtentry->dst_seq = tmp_unr_dst->unr_dst_seq;
	  tmp_rtentry->lst_hop_cnt = tmp_rtentry->hop_cnt;
	  tmp_rtentry->hop_cnt = 255;
	  if(tmp_rtentry->precursors->next->ishead != 1) 
	    {
	      /* precursors exist */
	      if(!new_rerr_created)
		{
		  new_rerr_created = 1;
		  if((new_rerrhdr = 
		      create_rerrhdr(tmp_unr_dst->unr_dst_ip,
				     tmp_unr_dst->unr_dst_seq)) == NULL)
		    return -1;
		}
	      else
		{
		  if((append_unr_dst(new_rerrhdr,
				     tmp_unr_dst->unr_dst_ip,
				     tmp_unr_dst->unr_dst_seq)) == -1)
		    return -1;
		}
	    }
	}
    }
  
  if(new_rerr_created)
    {
      send_rerr(tmp_info, new_rerrhdr);
      free_rerrhdr(new_rerrhdr);
    }

  return 0;
}


/*
 * create_rerrhdr
 *
 * Description: 
 *   create_rerrhdr is used to create a new RERR message
 *
 * Arguments: 
 *   u_int32_t tmp_ip - IP address for the broken destination
 *   u_int32_t tmp_dst_seq - sequence no for the broken destination.
 *
 * Returns: 
 *   struct rrerrhdr* - pointer to the new rerrhdr. 
 *                      NULL on failure.
 */
struct rerrhdr*
create_rerrhdr(u_int32_t tmp_ip, u_int32_t tmp_dst_seq)
{
  struct rerr_unr_dst *tmp_rerr_unr_dst;
  struct rerrhdr *tmp_rerrhdr;

  if((tmp_rerrhdr = malloc(sizeof(struct rerrhdr))) == NULL)
    return NULL;

  if((tmp_rerr_unr_dst = malloc(sizeof(struct rerr_unr_dst))) == NULL)
    return NULL;

  tmp_rerr_unr_dst->unr_dst_ip = tmp_ip;
  tmp_rerr_unr_dst->unr_dst_seq = tmp_dst_seq;
  tmp_rerr_unr_dst->next = NULL;
 
  tmp_rerrhdr->unr_dst = tmp_rerr_unr_dst;
  tmp_rerrhdr->type = 3;
  tmp_rerrhdr->dst_cnt = 1;

  return tmp_rerrhdr;
}


/*
 * append_unr_dst
 *
 * Description: 
 *   append_unr_dst adds an unreachable node to the previously
 *   created RERR message.
 *
 * Arguments: 
 *   struct rerrhdr *tmp_rerrhdr - Pointer to the rerrhdr to append to 
 *   u_int32_t tmp_ip - IP address for the broken destination
 *   u_int32_t tmp_dst_seq - sequence no for the broken destination
 *
 * Returns: 
 *   int - 0 on success
 *        -1 on failure
 */
int
append_unr_dst(struct rerrhdr *tmp_rerrhdr, u_int32_t tmp_ip, 
	       u_int32_t tmp_dst_seq)
{
  struct rerr_unr_dst *tmp_rerr_unr_dst;

  if((tmp_rerr_unr_dst = malloc(sizeof(struct rerr_unr_dst))) == NULL)
    return -1;

  tmp_rerr_unr_dst->next = tmp_rerrhdr->unr_dst;
  tmp_rerrhdr->unr_dst = tmp_rerr_unr_dst;
  tmp_rerrhdr->dst_cnt++;
  tmp_rerr_unr_dst->unr_dst_ip = tmp_ip;
  tmp_rerr_unr_dst->unr_dst_seq = tmp_dst_seq;

  return 0;
}


/*
 * free_rerrhdr
 *
 * Description: 
 *   free_rerrhdr frees the allocated memory for the rerrhdr
 *   list structure.
 *
 * Arguments: 
 *   struct rerrhdr *tmp_rerrhdr - Pointer to the rerrhdr to free.
 *
 * Returns: void
 */

void
free_rerrhdr(struct rerrhdr *tmp_rerrhdr)
{
  struct rerr_unr_dst *tmp_unr_dst;

  for(tmp_unr_dst = tmp_rerrhdr->unr_dst;
      tmp_rerrhdr->unr_dst != NULL;
      tmp_unr_dst = tmp_rerrhdr->unr_dst)
    {
      tmp_rerrhdr->unr_dst = tmp_unr_dst->next;
      free(tmp_unr_dst);
    }

  free(tmp_rerrhdr);
}


/*
 * route_expiry
 *
 * Description: 
 *   route_expiry invalidates an active route, i e an entry
 *   in the routing table.
 *
 * Arguments: 
 *   struct artentry *tmp_rtentry - Pointer to the entry
 *
 * Returns: void
 */
void
route_expiry(struct artentry *tmp_rtentry)
{
  tmp_rtentry->dst_seq++;
  tmp_rtentry->lst_hop_cnt = tmp_rtentry->hop_cnt;
  tmp_rtentry->hop_cnt = 255;
  tmp_rtentry->lifetime = (getcurrtime() + DELETE_PERIOD);
  
  del_kroute(tmp_rtentry->dst_ip, tmp_rtentry->nxt_hop);
}


/*
 * send_rerr
 *
 * Description: 
 *   send_rerr copies the incoming RERR message to a connected
 *   data area, which is a suitable format for the function send_datagram.
 *
 * Arguments: 
 *   struct info *tmp_info - info structure
 *   struct rrerhdr *tmp_rerrhdr - pointer to the rerrhdr to broadcast.
 *
 * Returns: 
 *   int - 0 on success
 *        -1 on failure.
 */
int
send_rerr(struct info *tmp_info, struct rerrhdr *tmp_rerrhdr)
{
  struct rerr_unr_dst *tmp_unr_dst;
  int datalen, i;
  char *data;

  tmp_info->ip_pkt_src_ip=tmp_info->ip_pkt_my_ip;
  tmp_info->ip_pkt_dst_ip=inet_addr("255.255.255.255");
 
  datalen = 4 + 8 * (tmp_rerrhdr->dst_cnt);
  if((data = malloc(datalen*sizeof(char))) == NULL)
    return -1;

  data[0] = 3;
  data[1] = 0;
  data[2] = 0;
  data[3] = tmp_rerrhdr->dst_cnt;
  for(i = 0, tmp_unr_dst = tmp_rerrhdr->unr_dst;
      i < tmp_rerrhdr->dst_cnt;
      i++, tmp_unr_dst = tmp_unr_dst->next)
    {
      memcpy(data + 4 + i * 8, &(tmp_unr_dst->unr_dst_ip), 4);
      memcpy(data + 8 + i * 8, &(tmp_unr_dst->unr_dst_seq), 4);
    }

  send_datagram(tmp_info, data, datalen);

  free(data);

  return 0;
}


/*
 * print_rerrhdr
 *
 * Description: 
 *   print_rerrhdr prints a RERR message
 *
 * Arguments: 
 *   struct rerrhdr *new_rerrhdr - Pointer to the rerrhdr
 *
 * Returns: void
 */
void
print_rerrhdr(struct rerrhdr *new_rerrhdr)
{
  struct rerr_unr_dst *tmp_rerr_unr_dst;
  struct in_addr tmp_in_addr;
  int i;

  printf("Outgoing RERR message: type: %d dst_cnt: %d\n",
	 new_rerrhdr->type, new_rerrhdr->dst_cnt);

  for(i = 0, tmp_rerr_unr_dst = new_rerrhdr->unr_dst;
      i < new_rerrhdr->dst_cnt;
      i++, tmp_rerr_unr_dst = tmp_rerr_unr_dst->next)
    {
      tmp_in_addr.s_addr = tmp_rerr_unr_dst->unr_dst_ip;
      printf("unr_dst_ip: %s unr_dst_seq %d\n",
	     inet_ntoa(tmp_in_addr),
	     tmp_rerr_unr_dst->unr_dst_seq);
    }
}
