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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "aodv.h"
#include "utils.h"
#include "rt_entry.h"
#include "info.h"
#include "RT.h"

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
int link_break(struct info* tmp_info, u_int32_t brk_dst_ip);

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
int host_unr(struct info* tmp_info, u_int32_t brk_dst_ip);

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
int rec_rerr(struct info *tmp_info, struct rerrhdr *tmp_rerrhdr);

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
struct rerrhdr* create_rerrhdr(u_int32_t tmp_ip, u_int32_t tmp_dst_seq);

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
int append_unr_dst(struct rerrhdr *tmp_rerrhdr,
		   u_int32_t tmp_ip, u_int32_t tmp_dst_seq);

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
void route_expiry(struct artentry* tmp_rtentry);

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
void print_rerrhdr(struct rerrhdr* tmp_rerrhdr);
