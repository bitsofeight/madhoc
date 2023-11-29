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
 *        Forks of a process scanning the device in use with AODV for
 *        incoming and outgoing packets. Information from these packets
 *        are sent to a pipe listened to by the main aodv_daemon process.
 *
 *        Uses the libpcap module.
 *
 *	Internal procedures:
 *        packetcapture()
 *        send_to_pipe()
 *	  scan_packets()
 *
 *	
 *	External procedures:
 *        packetcaptureinit()
 *
 ********************************
 *
 * Extendend RCS Info: $Id: gen_rreq.c,v 1.11 2000/04/18 07:54:15 root Exp root $
 *
 */

#ifndef PACKETCAP_H
#define PACKETCAP_H

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <fcntl.h>

#include "utils.h"
#include "RT.h"

#define SP_TYPE_IP   1
#define SP_TYPE_ARP  2
#define SP_TYPE_ICMP 3

#define MAXLINE   4096
#define READ         0
#define WRITE        1

struct scanpac
{
  unsigned char type;
  u_int32_t ip;
};


/* 
 *   packetcaptureinit
 *
 *   Description: 
 *     Initializes the interface to scan in promiscous mode. Several calls
 *     to libpcap are made, and a pipe is opened. A scanning process is
 *     forked  off, and the control is returned to the aodv_daemon process.  
 *
 *   Arguments:
 *     char *interface - The name of the interface to scan for packets.
 *
 *   Return:
 *     int - Returns 0 if sucessful. -1 if lib_pcap fails. -2 if fork fails.
 */
int packetcaptureinit(char *interface);

#endif















