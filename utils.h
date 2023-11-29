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
 *        Misc usefull fuctions
 *
 *	Internal procedures:
 *	
 *	External procedures:
 *        send_datagram
 *        getcurrtime
 *
 *
 ********************************
 *
 * Extendend RCS Info: $Id: utils.h,v 1.5 2000/04/17 14:11:10 root Exp root $
 *
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "info.h"
#include "aodv.h"
#include "timer.h"
#include "logmsg.h"


/* 
 *   send_datagram
 *
 *   Description: 
 *     Sends a datagram with the given input.
 *
 *   Arguments:
 *     struct info *pktinfo - Includes the address info needed to send the
 *                            package.
 *       pktinfo->ip_pkt_src_ip = pktinfo->ip_pkt_my_ip = The hosts own ip
 *       pktinfo->ip_pkt_dst_ip = The address to which the packets is sent
 *                                to, Can be broadcast (255.255.255.255).
 *       pktinfo->ip_pkt_ttl    = The ttl to send the packet with. If 0 the
 *                                system default ttl is used.
 *
 *     void *data           - The data to be sent.
 *     int datalen          - The length of the data to be sent.
 *
 *   Return: 
 *     int - On error -1 is returned else 0 is returned.
 */
int send_datagram(struct info *pktinfo,void *data,int datalen);

/* 
 *   getcurrtime
 *
 *   Description: 
 *     Gets the current time in milliseconds since 1 jan 1970
 *
 *   Arguments: None
 *
 *   Return: 
 *     u_int64_t - On error -1 is returned otherwise the time.
 */
u_int64_t getcurrtime();

#endif
