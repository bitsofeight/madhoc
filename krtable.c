/*      
 *      FILE: $RCSFile$ 
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
 *      General description:
 *         Interface for simpler modification of the kernel's routing table 
 *         from for instance the AODV routing table.
 *        
 *      Internal procedures:
 *         gen_krtentry(u_int32_t dst_ip, u_int32_t gw_ip)
 *
 *      External procedures:
 *         init_rtsocket()
 *         add_kroute(u_int32_t dst_ip, u_int32_t gw_ip)
 *         del_kroute(u_int32_t dst_ip, u_int32_t gw_ip)
 *
 ********************************
 *
 *      Extendend RCS Info: $Id$
 *
 */

#include"krtable.h"

/* 
 *  init_rtsocket
 *
 *  Description:
 *    Initiates the socket through which commands to the
 *    kernels routing table are given.
 * 
 *  Arguments:
 *  Return:  
 *  int - File descriptor to the kernel routing table
 *       -1 on failure
 */
int 
init_rtsocket()
{  
  int fd = 0;
  int type = SOCK_DGRAM;
  
  /* Get socket for UDP or TCP */
  if ((fd = socket(AF_INET, type, 0)) < 0)
    return -1; /* Unable to create socket */
  
  return fd;
}

/*
 * gen_krtentry
 *
 * Description:
 *   Generates an entry for the kernel's routing table to be used
 *   both att addition and deletion of routes.
 *
 * Arguments:
 *   u_int32_t dst_ip - IP address to the destination
 *   u_int32_t gw_ip  - IP address to gateway of the route
 *
 * Returns:
 *   struct rtentry* - pointer to the entry
 */
struct rtentry*
gen_krtentry(u_int32_t dst_ip, u_int32_t gw_ip)
{
  struct rtentry *new_rtentry;
  struct sockaddr_in dst;
  struct sockaddr_in gw;
  struct sockaddr_in genmask;
  
  if((new_rtentry = malloc(sizeof(struct rtentry))) == NULL)
    return NULL; /* Malloc failed */

  dst.sin_family = AF_INET;
  gw.sin_family = AF_INET;
  genmask.sin_family = AF_INET;
  
  dst.sin_addr.s_addr = dst_ip;
  gw.sin_addr.s_addr = gw_ip;
  genmask.sin_addr.s_addr = inet_addr("255.255.255.255");
  
  new_rtentry->rt_flags = RTF_UP | RTF_HOST | RTF_GATEWAY;
  new_rtentry->rt_metric = 0;
  new_rtentry->rt_dev = NULL;
  new_rtentry->rt_dst = *(struct sockaddr*) &dst;
  new_rtentry->rt_gateway = *(struct sockaddr*) &gw;
  new_rtentry->rt_genmask = *(struct sockaddr*) &genmask;

  return new_rtentry;
}

/*
 * add_kroute
 *
 * Description:
 *   Adds a route to the kernel's routing table.
 *
 * Arguments:
 *   u_int32_t dst_ip - IP address to the destination
 *   u_int32_t gw_ip  - IP address to the gateway of the route
 *
 * Returns:
 *   int - 0 on success
 *        -1 on failure
 */
int
add_kroute(u_int32_t dst_ip, u_int32_t gw_ip)
{
  struct rtentry  *new_krtentry;
  
  if ((new_krtentry = gen_krtentry(dst_ip, gw_ip)) == NULL)
    /* gen_krtentry failed */
    return -1; 
  
  if(ioctl(krt, SIOCADDRT, (char*) new_krtentry) == -1)
    /* SIOCADDRT failed */ 
    return -1;
  
  return 0;
}

/*
 * del_kroute
 *
 * Description:
 *   Deletes a route from the kernel's routing table.
 *
 * Arguments:
 *   u_int32_t dst_ip - IP address to the destination
 *   u_int32_t gw_ip  - IP address to the gateway of the route
 *
 * Return:
 *   int - 0 on success
 *        -1 on failure
 */ 
int
del_kroute(u_int32_t dst_ip, u_int32_t gw_ip)
{
  struct rtentry  *new_krtentry;

  if ((new_krtentry = gen_krtentry(dst_ip, gw_ip)) == NULL)
    /* del_kroute failed */
    return -1;
  
  if(ioctl(krt, SIOCDELRT, (char*) new_krtentry) == -1)
    /* SIOCDELRT failed */
    return -1;
  
  return 0;
}
