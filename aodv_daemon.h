/*	
 *	FILE: aodv_deamon.c	
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
 *      Usage: 
 *        <aodv_daemon_X_xx "interface"> where "interface" indicates the 
 *        name of the interface to be used, as shown in the ifconfig listing.
 *        X_xx indicates the RCS version number.
 *       
 ********************************
 *
 *	General description:
 *      -------------------
 *
 *
 * This file contains the main loop of the AODV-daemon. After initializing
 * the AODV port, the routing table, the RREQ drop-list, and the message 
 * handling, the program reads from several "files" using a select-statement.
 * Internal events are queued using a timer, and subsequently fed to a pipe
 * for processing. Incoming packets on the AODV port, and incoming events, 
 * are then dispatched to the corresponding event/packet-handling function. 
 *
 *	Internal procedures:
 *
 * parse_arguments()
 * init_socket()
 * bind_socket()
 * get_interface_ip()
 * initialize_RT()
 * check_packet()
 * init_anc_message()
 * make_info_struct()
 *	
 *	External procedures:
 *
 * main()
 *
 ********************************
 *
 *   Extendend RCS Info: $Id: aodv_daemon.c,v 1.68 2000/05/22 17:08:42 root Exp root $
 *
 */

#ifndef AODV_DAEMON_H
#define AODV_DAEMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <signal.h>

#include "aodv.h"
#include "info.h"
#include "rreq.h"
#include "rrep.h"
#include "rerr.h"
#include "find_inactives.h"
#include "RT.h"
#include "timer.h"
#include "to_rreq.h"
#include "gen_rreq.h"
#include "uio.h"
#include "logmsg.h"
#include "packetcap.h"

#define PRINT_RT_INTERVAL 2000

struct a_in_pktinfo
{
  unsigned int   ipi_ifindex;  /* Interface index */
  struct in_addr ipi_spec_dst; /* Routing destination address */
  struct in_addr ipi_addr;     /* Header Destination address */
};

union control_union 
{
  struct cmsghdr cm;
  char control[CMSG_SPACE(sizeof(int)) + 
	      CMSG_SPACE(sizeof(struct a_in_pktinfo))];
};

#endif
