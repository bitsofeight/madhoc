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

#include "logmsg.h"

/* 
 *   septime
 * 
 *   Description: 
 *     Writes the 64bit number in groups of two
 *     to the buffer, separated by :'s.
 *
 *   Arguments:
 *     char *buffer     - The buffer to be written in to
 *     u_int64_t u64num - The 64 bit number to be written
 *
 *   Return:
 *     char*            - Sames as the input buffer
*/
char *
septime(char *buffer, u_int64_t u64num)
{
  char tmpbuff[MAXBUFLEN];
  int i = 0, j = 0;
  
  sprintf(tmpbuff, "%Lu", u64num);
  for (i = 0;i < strlen(tmpbuff); i++)
    {
      buffer[j++] = tmpbuff[i];
      if ((i + 1) % 2 == 0 && (i + 1) != strlen(tmpbuff))
	buffer[j++] = ':';
    }
  buffer[j] = '\0';
  
  return buffer;
}

/* 
 *   print_rreq
 *
 *   Description: 
 *     Writes a RREQ package to the filedescriptor writefd
 *
 *   Arguments:
 *     struct rreq* pkt - The packet to be logged.
 *
 *   Return: None.
*/
void
print_rreq(struct rreq* pkt, int writefd)
{
  char outbuffer[MAXBUFLEN];  
  
  sprintf(outbuffer, 
	  "Type: %u(RREQ)  J: %u  R: %u  Reserved: %u  Hop Count: %u\n",
	  pkt->type,pkt->j, pkt->r, pkt->reserved, pkt->hop_cnt);
  write(writefd, outbuffer, strlen(outbuffer));
  
  sprintf(outbuffer, "BroadcastID: %u\n", pkt->broadcast_id);
  write(writefd, outbuffer, strlen(outbuffer));
  
  sprintf(outbuffer, "Dst IP: %s\tDst Seq nr: %u\n",
	  inet_ntoa(*((struct in_addr*)&pkt->dst_ip)), pkt->dst_seq);
  write(writefd, outbuffer, strlen(outbuffer));
  
  sprintf(outbuffer, "Src IP: %s\tSrc seq nr: %u\n",
	  inet_ntoa(*((struct in_addr*)&pkt->src_ip)), pkt->src_seq);
  write(writefd, outbuffer, strlen(outbuffer));
}

/* 
 *   print_rrep
 *
 *   Description: 
 *     Writes a RREP package to the filedescriptor writefd
 *
 *   Arguments:
 *     struct rrep* pkt - The packet to be logged.
 *
 *   Return: None.
*/
void
print_rrep(struct rrep* pkt, int writefd)
{
  char outbuffer[MAXBUFLEN];  
  
  sprintf(outbuffer, "Type: %u(RREP)  R: %u  Reserved: %u  " 
	  "Prefix Sz: %u  Hop Count: %u\n",
	  pkt->type,pkt->r, pkt->reserved, pkt->prefix_sz, pkt->hop_cnt);
  write(writefd, outbuffer, strlen(outbuffer));
  
  sprintf(outbuffer, "Dst IP: %s\tDst Seq nr: %u\n",
	  inet_ntoa(*((struct in_addr*)&pkt->dst_ip)), pkt->dst_seq);
  write(writefd, outbuffer, strlen(outbuffer));
  
  sprintf(outbuffer, "Src IP: %s\n",
	  inet_ntoa(*((struct in_addr*)&pkt->src_ip)));
  write(writefd, outbuffer, strlen(outbuffer));
  
  sprintf(outbuffer, "Lifetime: %u\n", pkt->lifetime);
  write(writefd, outbuffer, strlen(outbuffer));
}

/* 
 *   print_rerr
 *
 *   Description: 
 *     Writes a RERR package to the filedescriptor writefd
 *
 *   Arguments:
 *     void* pkt - The raw data to be interpreted as a RERR package.
 *
 *   Return: None.
*/
void
print_rerr(void *pkt, int writefd)
{
  char outbuffer[MAXBUFLEN];  
  struct rerrdst *m_rerrdst;
  struct rerr *m_rerr;
  struct rerr t_rerr;
  int i = 0;

  m_rerr = (struct rerr*)pkt;

  t_rerr.type = 3;
  t_rerr.reserved = 0;
  t_rerr.dst_cnt = 2;

  sprintf(outbuffer, "Type: %d(RRER)  Reserved: %u  Count: %u\n",
	  m_rerr->type, m_rerr->reserved, m_rerr->dst_cnt);
  write(writefd, outbuffer, strlen(outbuffer));

  for (i = 0 ; i < m_rerr->dst_cnt ; i++)
    {
      m_rerrdst = (struct rerrdst*)(pkt + sizeof(struct rerr) + 
				    (i * sizeof(struct rerrdst)));

      sprintf(outbuffer, "Dst IP: %s\tDst Seq nr: %u\n",
	      inet_ntoa(*((struct in_addr*)&m_rerrdst->unr_dst_ip)),
	      m_rerrdst->unr_dst_seq);
      write(writefd, outbuffer, strlen(outbuffer));
    } 
}


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
void
logmsg(void *data, int datalen, struct info *is)
{
  static int writefd = -1;

  char outbuffer[MAXBUFLEN];
  u_int64_t currtime;
  u_int8_t pkt_type;
  
  if (writefd == -1)
    {
      if ((writefd = open("logmsg.txt", 
			  O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU)) < 0)
	/*  Couldn't open logfile */
	return;
    }
  
  sprintf(outbuffer,"\n");
  write(writefd, outbuffer, strlen(outbuffer));
  
  /* Write timestamp */
  currtime = getcurrtime();
  septime(outbuffer, currtime);
  write(writefd, outbuffer, strlen(outbuffer));

  /* Write source dest and ttl */
  sprintf(outbuffer, " %s", 
	  inet_ntoa( *((struct in_addr*)&is->ip_pkt_src_ip) ));
  write(writefd, outbuffer, strlen(outbuffer));

  sprintf(outbuffer, " -> %s ttl: %d  len:%d \n",
	  inet_ntoa(*((struct in_addr*)&is->ip_pkt_dst_ip)), 
	  is->ip_pkt_ttl, datalen);
  write(writefd, outbuffer, strlen(outbuffer));
  
  /* Extract the package type (we know it is 8 bits)*/
  pkt_type = *((u_int8_t*)data);
  switch (pkt_type)
    {
    case(RREQ):
      print_rreq((struct rreq*)data, writefd);
      break;
      
    case(RREP):
      print_rrep((struct rrep*)data, writefd);
      break;
      
    case(RERR):
      print_rerr(data, writefd);
      break;
      
    default:
    }
}
