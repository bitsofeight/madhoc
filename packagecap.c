/*	
 *	FILE: $RCSFile$	
 *
 ********************************
 *
 *	General description:
 *        Forks of a process scanning the device in use with AODV for incoming and 
 *        outgoing packets. Information from these packets are sent to a pipe
 *        listened to by the main aodv_daemon process.
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
 *	Extendend RCS Info: $Id: gen_rreq.c,v 1.11 2000/04/18 07:54:15 root Exp root $
 *
 */

#include "packagecap.h"

extern u_int32_t g_my_ip;
extern int scanner_pid;

static pcap_t *pd;
static int datalink;
static int my_pipe[2];

/* 
 *   packetcaptureinit
 *
 *   Description: 
 *     Counts the number of occurences of 
 *     a character in a string.
 *
 *   Arguments:
 *     char *cs - A pointer to a string to search.
 *     char c   - The character to serach for.
 *
 *   Return:
 *     int - The number of occurences of c in cs
 */

int
packetcaptureinit(char *interface) 
{
  int snaplen = 100;

  char cmd[MAXLINE];
  char errbuf[PCAP_ERRBUF_SIZE];
  struct bpf_program fcode;
  bpf_u_int32 localnet;
  bpf_u_int32 netmask;
  int s1;

  s1 = pipe(my_pipe);

  switch ( scanner_pid = fork() ) 
    {
    case 0: /* packet capturer. child */
      signal(SIGINT, NULL);
      
      close(READ);      
      close(my_pipe[READ]);
      
      if((pd = pcap_open_live(interface, snaplen, 1, 0, errbuf)) == NULL)
	return(-1);
      
      if (pcap_lookupnet(interface, &localnet, &netmask, errbuf) < 0)
	return(-1);
      
      snprintf(cmd, sizeof(cmd),"arp or ip");
      
      if (pcap_compile(pd, &fcode, cmd, 0, netmask) < 0)
	return(-1);
      
      if (pcap_setfilter(pd, &fcode) < 0)
	return(-1);
      
      if ((datalink = pcap_datalink(pd)) < 0)
	return(-1);
      
      packetcapture(-1);
      break;
      
    case -1: /* Fork failed */
      return(-2);
      break;
      
    default: /* Daemon process. Parent. */
      close(my_pipe[WRITE]);
      
      return (my_pipe[READ]);
      break;
    }

  return(-1);
}

void
send_to_pipe(struct scanpac *sp) 
{
  write(my_pipe[WRITE], sp, sizeof(struct scanpac));
}

void
scan_packets(u_char *what_is_this, const struct pcap_pkthdr *hdr, const u_char *data)
{
  struct ether_header *eptr;
  struct ether_arp *arp;
  struct icmphdr *icmp;
  struct ip *ip;
  
  struct scanpac sp;
  
  switch(datalink)
    {
    case DLT_EN10MB:
      eptr = (struct ether_header*) data;
      switch (ntohs(eptr->ether_type))
	{
	case ETHERTYPE_IP:
	  ip = (struct ip*)(data + 14);
	  switch (ip->ip_p)
	    {
	    case (IPPROTO_ICMP):
	      icmp = (struct icmphdr*)(data + 14 + 20);
	      if (icmp->type == ICMP_DEST_UNREACH && 
		  (icmp->code == ICMP_UNREACH_HOST || 
		   icmp->code == ICMP_UNREACH_HOST_UNKNOWN))
		{
		  ip = (struct ip*)(data + 14 + 20 + 8);
		  sp.ip = ip->ip_dst.s_addr;
		  sp.type = SP_TYPE_ICMP;
		  send_to_pipe(&sp);
		  break;
		}
	      
	    default:
	      sp.ip = ip->ip_dst.s_addr;
	      sp.type = SP_TYPE_IP;
	      send_to_pipe(&sp);
	      break;
	    }
	  
	case ETHERTYPE_ARP:
	  arp = (struct ether_arp*)(data + 14);
	  
	  if (ntohs((arp->ea_hdr).ar_op) == ARPOP_REQUEST && 
	      ntohs((arp->ea_hdr).ar_hrd) == ARPHRD_ETHER && 
	      ntohs((arp->ea_hdr).ar_pro) == ETHERTYPE_IP) 
	    {
	      if ( (*((u_int32_t*)arp->arp_spa)) == g_my_ip )
		{
		  sp.ip = *((u_int32_t*)arp->arp_tpa);
		  sp.type = SP_TYPE_ARP;
		  send_to_pipe(&sp);
		  break;
		}
	    }
	}
    }
}

int
packetcapture(int maxpackages)
{
  int packets = 0;

  packets = pcap_loop(pd, maxpackages, scan_packets, NULL);

  return packets;
}















