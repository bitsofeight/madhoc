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
 * Extendend RCS Info: $Id: utils.c,v 1.6 2000/04/17 14:10:04 root Exp root $
 *
 */

#include "utils.h"

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
int
send_datagram(struct info *pktinfo, void *data, int datalen)
{
  /* Keep the socket open */
  struct sockaddr_in their_addr;
  int sentbytes;
  int sockfd;
  int on = 1;
  
  struct prioqent *my_pqe;
  struct rreq_tdata *trd;

#ifdef LOGMSG
	logmsg(data, datalen, pktinfo);
#endif
  

  if ((sockfd = socket(AF_INET, SOCK_DGRAM,0)) < 0)
    /* Error creating socket */
    return -1;
  
  /* Fill in destination of the package */
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(AODVPORT);
  their_addr.sin_addr.s_addr = pktinfo->ip_pkt_dst_ip;
  bzero(&(their_addr.sin_zero), 8);
  
  /* Is destination broadcast ? */
  if (their_addr.sin_addr.s_addr == inet_addr("255.255.255.255"))
    {
      /* We are sending broadcasts so dont send a hello until 
	 HELLO_INTERVAL later */
      my_pqe = pq_getfirstofidflags(inet_addr("255.255.255.255"), 
				    PQ_PACKET_HELLO);
      if (my_pqe != NULL)
	{
	  trd = (struct rreq_tdata*)my_pqe->data;
	  pq_deleteent(my_pqe);
	  pq_insert(getcurrtime() + HELLO_INTERVAL, trd, 
		    inet_addr("255.255.255.255"), PQ_PACKET_HELLO);
	}
      if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
	{
	  /* Error setting socket options */
	  close(sockfd);
	  return -1;
	}
    }
  
  /* Set the TTL ? */
  if (pktinfo->ip_pkt_ttl != 0 && pktinfo->ip_pkt_ttl < 256)
    {
      if (setsockopt(sockfd, SOL_IP, IP_TTL, &(pktinfo->ip_pkt_ttl),
		     sizeof(pktinfo->ip_pkt_ttl)) < 0)
	{
	  /* Error setting socket options */
	  close(sockfd);
	  return -1;
	}
    }
  
  /* Send package */
  if ((sentbytes = sendto(sockfd, data, datalen, 0, 
			  (struct sockaddr *)&their_addr,
			  sizeof(struct sockaddr))) < 0)
    {
      /* Failed to send datagram */
      close(sockfd);
      return -1;
    }
  
  close(sockfd);
  
  return 0;
}

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
u_int64_t
getcurrtime()
{
  struct timezone tz;
  struct timeval tv;
  
  if (gettimeofday(&tv, &tz) < 0)
    /* Couldn't get time of day */
    return -1;
  
  return ((u_int64_t)tv.tv_sec) * 1000 + ((u_int64_t)tv.tv_usec) / 1000;
}
