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

#include "packetcap.h"

/* Predeclaration of internal procedures */
int packetcapture(int maxpackets);
void send_to_pipe(struct scanpac *sp);
void scan_packets(u_char *unused, const struct pcap_pkthdr *hdr, 
		  const u_char *data);

extern u_int32_t g_my_ip;
extern int scanner_pid;

static pcap_t *pd;
static int datalink;
static int my_pipe[2];

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
      
      /* Opens the interface for capture */
      if((pd = pcap_open_live(interface, snaplen, 1, 0, errbuf)) == NULL)
	return(-1);
      
      /* Can the interface/network really be used to scan packets? */
      if (pcap_lookupnet(interface, &localnet, &netmask, errbuf) < 0)
	return(-1);
      
      /* Only listen to ARP and IP */
      snprintf(cmd, sizeof(cmd),"arp or ip");
      
      /* Compile the filter to be used. */
      if (pcap_compile(pd, &fcode, cmd, 0, netmask) < 0)
	return(-1);
      
      /* Run the compiled filter */
      if (pcap_setfilter(pd, &fcode) < 0)
	return(-1);
      
      if ((datalink = pcap_datalink(pd)) < 0)
	return(-1);
      
      packetcapture(-1);
      /* If this function call returns, an error has occured. Exit! */
      exit(-1);
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

/*
 *  send_to_pipe
 *
 *  Description:
 *    Sends structure scanned packat to pipe 
 *
 *  Arguments:
 *    struct scanpac *sp - Structure with the scanned packet to send to 
 *                         the pipe
 *
 *  Return:  Void
 */
void
send_to_pipe(struct scanpac *sp) 
{
  write(my_pipe[WRITE], sp, sizeof(struct scanpac));
}

/* 
 *   packetcaptureinit
 *
 *   Description: 
 *     This function is called from within lib_pcap to handle packets that 
 *     were scanned.
 *
 *     Information about IP packets, normal IP-data and ICMPs, and ARP
 *     packets is collected.This information is put into a struct, that is
 *     passed to send_to_pipe(). IP packets increase lifetime of existing
 *     routes. ICMP host unreachable and network result in RERR. ARPs result
 *     in RREQ.
 *
 *   Arguments:
 *     u_char *unused Unused feature of lib_pcap
 *     const struct pcap_pkthdr *hdr Information about the packet.
 *     const u_char *data The data of the captured packet.
 *
 *   Return:
 *     int - Returns 0 if sucessful. -1 if lib_pcap fails. -2 if fork fails.
 */
void
scan_packets(u_char *unused, const struct pcap_pkthdr *hdr, 
	     const u_char *data)
{
  struct ether_header *eptr;
  struct ether_arp *arp;
  struct icmphdr *icmp;
  struct ip *ip;
  
  struct scanpac sp;
  
  switch(datalink)
    {
      /* Ethernet type? */
    case DLT_EN10MB:
      eptr = (struct ether_header*) data;
      switch (ntohs(eptr->ether_type))
	{
	  /* IP? */
	case ETHERTYPE_IP:
	  ip = (struct ip*)(data + 14);
	  switch (ip->ip_p)
	    {
	      /* IP of ICMP type? */
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
	      
	      /* Data packet. */
	    default:
	      sp.ip = ip->ip_dst.s_addr;
	      sp.type = SP_TYPE_IP;
	      send_to_pipe(&sp);
	      break;
	    }
	  
	  /* ARP packet? */
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

/* 
 *   packetcapture
 *
 *   Description: 
 *     This function is called to initialize lib_pcap: scan_packets() is 
 *     set to be the packet handling function. The pcap_loop call should
 *     never return.
 *
 *   Arguments:
 *     int maxpackets Always set to -1, which indicates "Scan forever".
 *
 *   Return:
 *     int - Returns -1 if lip_pcap fails or exits. Should never return if
 *           all is ok.
 */
int
packetcapture(int maxpackets)
{
  int packets = 0;

  packets = pcap_loop(pd, maxpackets, scan_packets, NULL);

  return -1;
}















