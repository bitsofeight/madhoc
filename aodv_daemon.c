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
 * Extendend RCS Info: $Id: aodv_daemon.c,v 1.68 2000/05/22 17:08:42 root Exp root $
 *
 */

#include "aodv_daemon.h"

#define MAXBUFLEN 1024

struct artentry *g_my_entry;
u_int32_t        g_my_ip;
int              scanner_pid;
int              reboot_state = 0;


/* 
 * parse_arguments
 * ---------------
 *
 * Description: 
 *   Gets the name of the interface intended for use with the AODV-daemon.
 *
 * Arguments: 
 *   <argc> and <argv> directly from main().
 *
 * Return: 
 *   Puts the only argument in <IF> if successful. 
 *   If not -1 is returned.
*/

char* 
parse_arguments (int argc, char *argv[])
{
  char *IF;
  
  if (argc != 2)
    /* Wrong number of arguments */
    return(NULL);
  
  /* Copy the interface from the command line */
  if ((IF = malloc(strlen(argv[1]) * sizeof(char))) == NULL)
    return(NULL);

  strncpy(IF,argv[1],strlen(argv[1]));

  return(IF);
}

/* 
 * check_root
 * ---------------
 *
 * Description: 
 *   Checks that the user running this program has uid and effective uid root (0)
 *
 * Arguments: 
 *   none
 *
 * Return: 
 *    0 if userid and effective user id is root(0)
 *   -1 if other
*/

int
check_root(void){
  if (getuid() || geteuid())
    return -1;
  
  return 0;
}

/* 
 * set ip forwarding
 * ---------------
 *
 * Description: 
 *   Tires to turn on ip forwarding in the kernel.
 *
 * Arguments: 
 *   none
 *
 * Return: 
 *    0 if it it successfull in turning on ipv4_forwarding 
 *   -1 if not
*/

int 
set_ipforwarding(void)
{
  int fd;
  char on = '1';
  
  if ((fd = open("/proc/sys/net/ipv4/ip_forward", O_WRONLY)) == -1){
    return -1;
  }
  
  if (write(fd,&on,sizeof(on)) == -1){
    printf("test2\n");
    return -1;
  }
  
  close(fd);
  return 0;
}               



/* 
 *   init_socket
 *   -----------
 *
 *  Description: 
 *    Gets a socket of type <type>. Tells this socket to:
 *      - Receive ancilliary message IP-packet TTL
 *      - Not to listen to loopback
 *      - Enable reuse of the address
 *      - Listen to broadcast
 * 
 *  Arguments:
 *    char *IF - Which interface to use
 *
 *  Return:
 *    int - File descriptor for the socket is returned if succeeded.
 *          If not returns -1.
 */
int 
init_socket (char *IF)
{
  int fd;
  int on = 1;
  int ttl = 1;
  int b_cast = 1;
  int type = SOCK_DGRAM;
  
  /* Get socket for UDP or TCP */
  if ((fd = socket(AF_INET, type, 0)) == -1)
    /* Error creating socket */
    return -1;
  
  /* Tell socket to send IP_TTL for received messages. */
  if (setsockopt(fd, SOL_IP, IP_RECVTTL, &ttl, sizeof(ttl)) < 0)
    /* Error in setting socket */
    return -1;
  
  /* Tell socket to send PKTINFO for received messages. */
  if (setsockopt(fd, SOL_IP, IP_PKTINFO, &on, sizeof(on)) < 0)
    /* Error in setting socket */
    return -1;
  
  /* Setting socket to only listen to out device (and not loopback) */
  if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, IF, 
		 (size_t)((strlen(IF)+1)*sizeof(char))) < 0)
    /* Error in setting socket */
    return -1;
  
  /* Setting socket so that more than one process can use the address */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    /* Error in setting socket */
    return -1;
  
  /* Listen to broadcast as well? */
  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &b_cast, sizeof(b_cast)) < 0)
    /* Error in setting socket */
    return -1;

  return fd;
}

/* 
 * get_interface_ip
 * ----------------
 *
 * Description:
 *   Parses the interface configuration. The config. is checked for each 
 *   interface (ensured by n = ifc.ifc_len/sizeof(struct ifreq). If a 
 *   non-internet address in ecountered it's ignored. If the name 
 *   corresponding to <IF> is found <addr> is set accordingly. If no
 *   matching ip-address is found an error value is retuned. 
 *   Also sets the global variable g_my_ip.
 *
 * Arguments: 
 *   int fd - File descriptor specifies which socket should be checked 
 *            for interfaces.
 *   char *IF - The name of the interface <IF> whose IP is looked up.
 *   int port - The port <port> used for the address.
 *   struct sockaddr_in *addr  - The address <addr> to be filled in.
 * 
 * Return: 
 *   int - Returns a completed <addr> and 0 if successful
 *         if not -1 is returned.
 */
int 
get_interface_ip (int fd, char *IF, int port, struct sockaddr_in *addr)
{
  struct sockaddr *sockaddrp;
  struct ifreq  *interfacep;
  struct sockaddr_in *addrp;
  struct ifconf ifc; 
  char buf[500];
  int n;

  /* Get interface configuration */
  ifc.ifc_len = sizeof (buf);   
  ifc.ifc_buf = buf; 
  if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) < 0)
    /* Error in ioctl */
    return(-1);

  interfacep = ifc.ifc_req;
  
  /* Parse config. for all interfaces */
  for (n = ifc.ifc_len / sizeof (struct ifreq); --n >= 0; interfacep++)
    { 
      if (interfacep->ifr_addr.sa_family != AF_INET)
	continue;
      
      sockaddrp = &(interfacep->ifr_addr);
      addrp = (struct sockaddr_in*)sockaddrp;
      
      if (strncmp(interfacep->ifr_name, IF, strlen(IF)) == 0)
	addr->sin_addr = addrp->sin_addr;
      else 
	continue;
    }

  /* Sets global variable g_my_ip */
  g_my_ip = addr->sin_addr.s_addr;
  
  return (0);
}

/* 
 *   bind_socket
 *   -----------
 *
 *  Description: 
 *    Binds the socket <fd> to the address <addr>.
 *
 *  Arguments:
 *    File descriptor for the socket to be bound. Address struct specifying
 *    what port and what ip-address should be used.
 *
 *  Return: 
 *    int- Zero if succeeded
 *        -1 if failed
 */

int
bind_socket (int fd, struct sockaddr_in *addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = htons(1303);
  bzero(&(addr->sin_zero), 8);
  addr->sin_addr.s_addr=INADDR_ANY;
  
  /* bind aodv socket to my address */
  if (bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr)) == -1)
    /* Error binding socket */
    return -1;
  
  return 0;  
}

/* 
 * check_packet
 * ------------
 *
 * Description:
 *   Checks the size of incoming packets and matches this with the stated 
 *   message type, and in the case of RERR with the number of error messages.
 *
 * Arguments:
 *   The number of bytes received <numbytes>, the packet type <type> and
 *   the number of unreachable destinations in an RERR packet <errcount>.
 *
 * Return:
 *   int - 0 if the packet is correct
 *        -1 if it's not.
 *
 */

int 
check_packet (int numbytes, int type, int errcount)
{
  switch (type) 
    {
    case 1: 
      if (numbytes == 24)
	return 0;
      break;
      
    case 2: 
      if (numbytes == 20) 
	return 0;
      break;
      
    case 3: 
      if (numbytes == (4 + 8 * errcount)) 
	return 0;
      break;
    
    default: 
      break;
    }

  return -1;
}


/* 
 * initialize_RT
 * -------------
 *
 * Description: 
 *   Initializes the routing table by calling init_rt(). Inserts entry to 
 *   the own node by setting the IP to my_addr. The destination sequence 
 *   number for this entry is used as source sequence number. Life time 
 *   is set to infinity,
 *   Also sets the global variable g_my_entry.
 *
 * Arguments: 
 *   My own address <addr> used to add my own entry.
 *
 * Return: 
 *   int - 0 if init_rt() and inert_entry() succeeds
 *         Otherwise -1 is returned.
 */
int
initialize_RT (struct sockaddr_in addr)
{
  struct artentry* rte;

  if (init_rt() == -1)
    /* Failed to initialize the routing table */
    return(-1);
  
  if ((rte = insert_entry()) == NULL)
    /* Couldn't create a new entry in the routing table */
    return(-1);
  
  rte->dst_ip = addr.sin_addr.s_addr;
  rte->dst_seq = 1;
  rte->broadcast_id = 1;
  rte->hop_cnt = 0;
  rte->lst_hop_cnt = 0;

  rte->nxt_hop = addr.sin_addr.s_addr;
  rte->lifetime = -1;
  rte->rt_flags = 0;
  
  /* Sets global variable <g_my_entry> : pointer to my entry */
  g_my_entry = rte;
  
  return 0;
}

/* 
 * init_anc_message
 * ----------------
 *
 * Description: 
 *   (Re)Initializes the ancilliary message for each loop passing the 
 *   select statement.
 * 
 * Arguments: 
 *   <msgh> is contructed every time. The control union <ctrl_un> is used
 *   to pad the data area passed to and from the message handler in the 
 *   kernel.
 *
 * Return: 
 *   The <msgh> is "nulled" each time. Always returns void.
 */
void
init_anc_message (struct msghdr *msgh, union control_union *ctrl_un)
{
  msgh->msg_name = NULL;
  msgh->msg_namelen = 0;
  msgh->msg_iov = NULL;
  msgh->msg_iovlen = 0;
  msgh->msg_control = ctrl_un->control;
  msgh->msg_controllen = sizeof(ctrl_un->control);
}

/* 
 * make_info_struct
 * ----------------
 *
 * Description: 
 *   Completes the info struct used by the various functions for processing
 *   packets/events. This struct is also used when sending datagrams and
 *   when logging messages.
 *
 * Arguments: 
 *   The senders address <ta>, the ip-packets destination address <pi>, 
 *   and the ttl <ttl> are used to fill in the info struct <inf>.
 *
 * Return: 
 *   The info struct <inf> is changed on every pass. The function 
 *   returns void.
 */

void
make_info_struct (struct sockaddr_in *ta, struct sockaddr_in *my, 
		  struct a_in_pktinfo *pi, int ttl, struct info *inf)
{
  inf->ip_pkt_src_ip = ta->sin_addr.s_addr;
  inf->ip_pkt_dst_ip = pi->ipi_addr.s_addr;
  inf->ip_pkt_my_ip = my->sin_addr.s_addr;
  inf->ip_pkt_ttl = ttl;
}

/* 
 * start_HELLO
 * -----------
 * Description: 
 *   Initiates HELLO message sending by calling gen_rreq() with
 *   dst_ip = broadcast.
 *
 * Arguments: None
 *
 * Return:
 *   int - Returns 0 if gen_rreq() ok 
 *         else return -1.
 */

int
start_HELLO ()
{
  struct info inf;

  inf.ip_pkt_dst_ip = inet_addr("255.255.255.255");
  inf.ip_pkt_src_ip = g_my_ip;
  inf.ip_pkt_my_ip = g_my_ip;
  inf.ip_pkt_ttl = 1;
  if (gen_rreq(&inf) == -1)
    /* Couldn't generate the RREQ for HELLO */
    return(-1);
  
  return 0;
}

/*
 * cleanup
 *
 * Description:
 *   Cleans up the lock files
 *
 * Arguments:
 *   int dummy - Argument passed on by SIGINT
 *
 * Returns: void
 */
void 
cleanup (int dummy)
{
  printf("Closing down...\n");
  kill(scanner_pid, SIGINT);
  krt_cleanup();

  remove("/var/lock/aodv_time");
  dummy = open("/var/lock/aodv_time", (O_CREAT | O_WRONLY));
  close(dummy);
  remove("/var/lock/aodv_lock");

  exit(0);
}

/*
 * reboot_wait
 *
 * Description:
 *   Makes sure the daemon waits enough time after reboot
 * 
 * Arguments:
 *   char *interface - which intreface the daemon should run on
 *   long wait - how long should it wait after reboot
 *   int aodvFD - file descriptor to the lock file
 *
 * Return: Void
 */
void
reboot_wait (char *interface, long wait, int aodvFD)
{
  int pipeFD;
  
  struct scanpac scanned_reboot;
  struct info info_msg_reboot;
  struct timeval tv_reboot;
  char buffer[MAXBUFLEN];
  fd_set readfds_reboot;
  int addr_len;
  
  switch (pipeFD = packetcaptureinit(interface))
    {
    case -1:
      printf("Packet capture init. Reboot\n");
      exit(1);
      break;
      
    case -2:
      printf("Fork failed. Reboot\n");
      exit(1);
      break;
      
    default:
    }

  while((wait - time(NULL)) > 0)
    {
      tv_reboot.tv_sec = 1;
      tv_reboot.tv_usec = 1;
      
      FD_ZERO(&readfds_reboot);
      FD_SET(pipeFD, &readfds_reboot); 
      FD_SET(aodvFD, &readfds_reboot);
      
      addr_len = sizeof(struct sockaddr);

      if(select(pipeFD + 1, &readfds_reboot, NULL, NULL, &tv_reboot) >= 0)
	{
	  if (FD_ISSET(pipeFD, &readfds_reboot))
	    {
	      /* Information from packet scanner arrived in pipe. Reboot mode
	       * ALL incoming data packets should result in an RERR.
	       */
	      
	      read(pipeFD, &scanned_reboot, sizeof(struct scanpac));
	      
	      switch (scanned_reboot.type)
		{
		case SP_TYPE_IP:
		  /* Send RERR for all packets received except broadcast */
		  if(scanned_reboot.ip != -1)
		    {
		      wait = time(NULL) + DELETE_PERIOD / 1000;
		      
		      info_msg_reboot.ip_pkt_dst_ip = scanned_reboot.ip;
		      info_msg_reboot.ip_pkt_src_ip = g_my_ip;
		      info_msg_reboot.ip_pkt_my_ip = g_my_ip;
		      info_msg_reboot.ip_pkt_ttl = 1;
		      host_unr(&info_msg_reboot,scanned_reboot.ip);
		    }
		  break;
		} 
	    }
	  else if (FD_ISSET(aodvFD, &readfds_reboot))
	    {
	      
	      /* AODV packet received. Silently discard */
	      recvfrom(aodvFD, buffer, MAXBUFLEN, 0, NULL, &addr_len);
	    }
	}
    }
  
  reboot_state = 0;
  kill(scanner_pid, SIGKILL);
  
  return;
}

/* ------------------------------------------------------------------- */

int 
main (int argc,char *argv[])
{
  /*
   * ---------------------------
   * Variable declarations:
   * ---------------------------
   */

  /* File descriptor for AODV, fd for packet scanner, IP-addresses */
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  char *interface;
  int addr_len;
  int aodvFD;
  
  /* Timer struct & file descriptor set for select */
  struct timeval tv;
  fd_set readfds;

  /* Buffer for recvfrom() */
  char buffer[MAXBUFLEN];
  int numbytes;

  /* Message for recvmsg() - used to get TTL and IP-packet info */
  struct cmsghdr *cmsg;
  struct msghdr msgh;
  int cmsgi;

  int *ttlptr;
  int received_ttl = 0;

  struct a_in_pktinfo pktinfo;
  union control_union control_un;

  /* Create aodv message types */
  struct rerrhdr rerrhdr_msg;
  struct rerr_unr_dst *tp;
  struct rreq *rreq_msgp;
  struct rrep *rrep_msgp;
  u_int8_t aodv_type;
  int rerri;

  /* Create info struct */
  struct info info_msg;

  /* Timer variables */
  struct prioqent *timer_pqe;
  int timerFD;

  /* Io types */
  struct info io_info;
  char *iobuff;

  /* Packet scanner pipe */
  struct artentry *scanned_rt;
  struct scanpac scanned;
  int pipeFD;
  int maxFD;

  /* Print RT */
  
  u_int64_t next_print;

  /* REBOOT */

  struct stat file_stat;
  long reboot_time = 0;
  int lockFD = 0;
  int timeFD = 0;

  /** End of variable declaration. **/


  /*
   * ---------------------------
   * Initialization:
   * ---------------------------
   */

  if ((interface = parse_arguments(argc, argv)) == NULL)
    {
      printf("Usage: aodv_daemon <interface>\n");
      exit(1);
    }

  /* See that we have the required privileges */
  if(check_root() == -1){
    printf("You must be root to run this program\n");
    exit(1);
  }

  /* See that we have ip_forwarding turned on */
  if(set_ipforwarding() == -1){
    printf("You must have ip forwarding enabled to run this program\n");
    exit(1);
  }

  /* Init Timer queue */
  if ((timerFD = pq_new()) == -1)
    {
      printf("Error initializing timer queue\n");
      exit(1);
    }
  
  /* Init RREQ list */
  if (init_rreq_list() == -1)
    {
      printf("Error initilizing RREQ list\n");
      exit(1);
    }

  /* Get socket */
  if ((aodvFD = init_socket(interface)) == -1)
    {
      printf("Error initializing socket\n");
      exit(1);
    }

  /* Bind socket */
  /* Why can't I bind to a specified address??? */
  if (bind_socket(aodvFD, &my_addr) == -1)
    {
      printf("Error binding socket\n");
      exit(1);
    }
  
  /* Get ip of interface card and create the my_addr struct. 
     Also set g_my_ip. */
  if (get_interface_ip(aodvFD, interface, AODVPORT, &my_addr) == -1)
    {
      printf("Error getting Interface IP\n");
      exit(1);
    }
  
  /* Initalize RT. Create my_entry, and dummy entry  */
  if (initialize_RT(my_addr) == -1)
    {
      printf("Error initializing routing table\n");
      exit(1);
    }

  /* Check if the daemon has crashed, or if a wait-state is necessary. */
  signal(SIGINT, (void*)cleanup);
  if ((lockFD = open("/var/lock/aodv_lock",
		     (O_RDONLY | O_CREAT | O_EXCL ))) == -1 )
    {
      reboot_state = 1;
      reboot_time = time(NULL) + DELETE_PERIOD/1000;
      close(lockFD);
    } 
  else if ((timeFD = open("/var/lock/aodv_time", O_RDONLY)) != -1)
    {
      stat("/var/lock/aodv_time", &file_stat);
      if( (time(NULL) - file_stat.st_ctime) < DELETE_PERIOD/1000)
	{
	  reboot_state = 1;
	  reboot_time = DELETE_PERIOD/1000 + file_stat.st_ctime;
	}
      close(timeFD);
    }   
  
  if (reboot_state)
    reboot_wait(interface, reboot_time, aodvFD);
  
  /* Starting HELLO-message timer */
  if (start_HELLO(info_msg) == -1)
    {
      printf("Error starting HELLO\n");
      exit(1);
    }

  /* Initialize packet capture */
  switch (pipeFD = packetcaptureinit(interface))
    {
    case -1:
      printf("Error initializing packet capture\n");
      exit(1);
      break;
      
    case -2:
      printf("Fork failed.\n");
      exit(1);
      break;
      
    default:
    }
  
  /* get the largest FD for select */
  maxFD = MAX ( MAX(aodvFD, timerFD), MAX(IO_FD, pipeFD)); 
  
  
  /*
   * ---------------------------
   * Main program loop:
   * ---------------------------
   */
  
  next_print = getcurrtime() + PRINT_RT_INTERVAL;
  
  while(1)
    {      
      
      find_inactives();

      if( next_print < getcurrtime() )
	{
	  next_print = getcurrtime() + SECOND * 2;
	  print_rt();
	}

      addr_len = sizeof(struct sockaddr);
      
      /* Note: the message msgh, the timer struct, and the FD_set must 
	 be set for each loop. */
      /* Initialize ancilliary message header for TTL and address options */
      
      init_anc_message(&msgh, &control_un);
      
      /* Set the timer struct used by select & add aodvFD to readFDset */
      tv.tv_sec = 1;
      tv.tv_usec = 1;
      FD_ZERO(&readfds);
      FD_SET(aodvFD, &readfds); 
      FD_SET(timerFD, &readfds);
      FD_SET(IO_FD, &readfds);
      FD_SET(pipeFD, &readfds);
      
      /* Wait until packet arrives or timer has run out! 
	 If select returns -1, a timer has expired. */
      if(select(maxFD + 1, &readfds, NULL, NULL, &tv) >= 0)
	{
	  /* Check if aodvFD has received a packet, or was it a timeout? */
	  if (FD_ISSET(aodvFD, &readfds))
	    {      
	      if (recvmsg (aodvFD, &msgh, MSG_PEEK) == -1)
		{
		  printf("Error recmsg");
		  exit(1);
		}
	      
	      if ((numbytes = recvfrom(aodvFD, buffer, MAXBUFLEN, 0, 
				     (struct sockaddr *)&their_addr,
				       &addr_len)) == -1)
		{
		  printf("Receive error!");
		  exit(1);
		}
	      buffer[numbytes] = '\0';      
	      
	      /* Dump all packets send from my own node.
		 Prevents bouncing messages. */
	      if (their_addr.sin_addr.s_addr == my_addr.sin_addr.s_addr)
		continue;
	      
	      /* Get the first message (of the two that should be received)
	       * then check if it's of the correct size. Parse the two
	       * messages. 
	       * If it's the TLL, set <received_ttl>. If it's the IP packet
	       * info, copy the relevant data to <pktinfo>. After each
	       * operation the cmsg in moved forward.
	       */
	      
	      cmsg = CMSG_FIRSTHDR(&msgh);
	      if (msgh.msg_controllen == 40)
		{
		  for (cmsgi = 0; cmsgi < 2; cmsgi++)
		    {
		      if (cmsg->cmsg_level == SOL_IP && 
			  cmsg->cmsg_type == IP_TTL)
			{
			  ttlptr = (int *) CMSG_DATA(cmsg);
			  received_ttl = *ttlptr;
			  cmsg = (void*)cmsg + CMSG_SPACE(sizeof(int));
			}
		      else if (cmsg->cmsg_level == SOL_IP && 
			       cmsg->cmsg_type == IP_PKTINFO)
			{
			  memcpy(&pktinfo, CMSG_DATA(cmsg),
				 sizeof(struct a_in_pktinfo));
			  cmsg = (void*)cmsg + 
			    CMSG_SPACE(sizeof(struct a_in_pktinfo));
			}
		    }
		}
	      
	      make_info_struct(&their_addr, &my_addr, &pktinfo,
			       received_ttl, &info_msg);
	      
#ifdef LOGMSG
	      logmsg(buffer, numbytes, &info_msg);
#endif
	      
	      /* What type of aodv message? */
	      aodv_type = (int)buffer[0];
	      switch (aodv_type)
		{
		case RREQ:
		  /* RREQ */
		  /* Cast to struct rreq */
		  if (check_packet(numbytes, aodv_type, 0) == -1)
		    break;
		  
		  rreq_msgp = (struct rreq*)buffer;
		  
		  rec_rreq(&info_msg, rreq_msgp);
		  break;
		  /* case 1 */
		  
		case RREP:
		  /* RREP */
		  /* Cast to struct rrep */
		  if (check_packet(numbytes, aodv_type, 0) == -1)
		    break;
		  
		  rrep_msgp = (struct rrep*)buffer;
		  rec_rrep(&info_msg, rrep_msgp);
		  break;
		  /* case 2 */
		  
		case RERR:
		  /* RERR */
		  /* Create header */
		  /* Must know the number of unreachable destinations 
		     before checking! */
		  
		  rerrhdr_msg.type = (u_int8_t)buffer[0];
		  rerrhdr_msg.reserved = (u_int16_t)buffer[1];
		  rerrhdr_msg.dst_cnt = (u_int8_t)buffer[3];
		  if (check_packet(numbytes, aodv_type,
				   rerrhdr_msg.dst_cnt) == -1)
		    break;
		  
		  /* Make space for dest_count structs, and assign them.
		   * This is a linked list of structs. The corresponding
		   * data from <buffer> is copied with memcopy. */
		  
		  rerrhdr_msg.unr_dst = NULL;
		  for (rerri = 0; rerri < rerrhdr_msg.dst_cnt; rerri++)
		    {
		      if ((tp = (struct rerr_unr_dst*) 
			   malloc(sizeof(struct rerr_unr_dst))) == NULL)
			{
			  break; /* Skip to next package */
			}
		      
		      tp->next = rerrhdr_msg.unr_dst;
		      rerrhdr_msg.unr_dst = tp;
		      memcpy((void *)&(tp->unr_dst_ip), 
			     (void *)&(buffer[4 + rerri * 8]), 4);
		      memcpy((void *)&(tp->unr_dst_seq), 
			     (void *)&(buffer[4 + rerri * 8+ 4 ]), 4);
		    }
		  
		  rec_rerr(&info_msg, &rerrhdr_msg);
		  
		  /* Free the list of structs that was sent to rec_rerr() */
		  for (rerri = 0; rerri < rerrhdr_msg.dst_cnt; rerri++)
		    {
		      tp = rerrhdr_msg.unr_dst;
		      rerrhdr_msg.unr_dst = rerrhdr_msg.unr_dst -> next;
		      free(tp);
		    }
		  break;
		  
		default:
		  /* Unknown message received on aodv-port */
		  
		} 
	    } 
	  
	  else if (FD_ISSET(timerFD, &readfds))
	    {
	      timer_pqe = pq_readpipe();
	      switch (timer_pqe->flags)
		{
		case PQ_PACKET_RREQ:
		  if (rreq_timeout(timer_pqe->data) == -1)
		    free(timer_pqe); /* Queue entry */
		  break;
		  
		case PQ_PACKET_HELLO:
		  hello_resend(timer_pqe->data);
		  break;
		  
		default:
		  break;
		}
	    }
	  
	  else if (FD_ISSET(IO_FD, &readfds))
	    {
	      /* User interactive input */
	
	      iobuff = io_read();
	      if (iobuff != NULL)
		{
		  io_info.ip_pkt_src_ip = my_addr.sin_addr.s_addr;
		  io_info.ip_pkt_my_ip = my_addr.sin_addr.s_addr;
		  io_parse(iobuff, &io_info);
		}
	    }
	  
	  else if (FD_ISSET(pipeFD, &readfds))
	    {
	      /* Information from packet scanner arrived in pipe */
	      
	      read(pipeFD, &scanned, sizeof(struct scanpac));
	      
	      switch (scanned.type)
		{
		case SP_TYPE_IP:
		  if (scanned.ip != g_my_ip)
		    {
		      if ((scanned_rt = getentry(scanned.ip)) != NULL)
			scanned_rt->lifetime = MAX(scanned_rt->lifetime, 
						   getcurrtime() + 
						   ACTIVE_ROUTE_TIMEOUT);
		    }
		  break;
		  
		case SP_TYPE_ARP:
		  scanned_rt = getentry(scanned.ip);
		  if (scanned_rt == NULL || scanned_rt->hop_cnt == 255)
		    {
		      info_msg.ip_pkt_dst_ip = scanned.ip;
		      info_msg.ip_pkt_src_ip = g_my_ip;
		      info_msg.ip_pkt_my_ip = g_my_ip;
		      info_msg.ip_pkt_ttl = 1;
		      gen_rreq(&info_msg);
		    }
		  break;
		  
		case SP_TYPE_ICMP:
		  info_msg.ip_pkt_dst_ip = scanned.ip;
		  info_msg.ip_pkt_src_ip = g_my_ip;
		  info_msg.ip_pkt_my_ip = g_my_ip;
		  info_msg.ip_pkt_ttl = 1;
		  if (pq_getfirstofidflags(scanned.ip, 
					   PQ_PACKET_RREQ) == NULL)
		    host_unr(&info_msg, scanned.ip);
		}
	    } 
	  
	} 
    }
} /* End of main */
