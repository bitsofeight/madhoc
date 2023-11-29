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

#ifndef GEN_RREQ_H
#define GEN_RREQ_H

#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "RT.h"
#include "rt_entry.h"
#include "info.h"
#include "aodv.h"
#include "utils.h"
#include "timer.h"
#include "to_rreq.h"
#include "rreq_list.h"

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
gen_rreq(struct info *is);

#endif
