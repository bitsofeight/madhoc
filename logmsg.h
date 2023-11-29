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
 *        Writes logging messages about packages to a filedescriptor
 *
 *     Internal procedures:
 *        septime
 *        print_rreq
 *        print_rrep
 *        print_rerr
 *
 *	External procedures:
 *        logmsg
 *
 ********************************
 *
  Extendend RCS Info: $Id: logmsg.c,v 1.3 2000/04/18 07:43:33 root Exp root $
 *
 */

#ifndef LOGMSG_H
#define LOGMSG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#include "aodv.h"
#include "utils.h"

/* The type field in the aodv packages */
#define RREQ         1
#define RREP         2
#define RERR         3

/* The filedescriptor to write to */
#define WRITE        1

#define MAXBUFLEN 1024


/* 
 *   logmsg
 *
 *   Description: 
 *     Takes a raw data package, determines the type of package
 *     and prints the data in the package.
 *
 *   Arguments:
 *     void *data      - The raw data area to be interpreted.
 *     int datalen     - The length of the raw data
 *     struct info *is - Information about the addresses related to the 
 *                       package.
 *       is->ip_pkt_dst_ip = To whome the package was sent (own ip or 
 *                           broadcast)
 *       is->ip_pkt_src_ip = Who sent the package.
 *       ip->ip_pkt_ttl    = The recived ttl.
 *       ip->ip_pkt_my_ip  = The host own ip.
 *
 *   Return: None.
*/
void logmsg(void *data, int datalen, struct info *is);

#endif

