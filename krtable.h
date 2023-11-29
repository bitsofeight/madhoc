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

#ifndef KRTABLE_H
#define KRTABLE_H

#include<net/if.h>
#include<net/route.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<sys/file.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

/* Declaration of global variable */
int krt;

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
int init_rtsocket ();

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
int add_kroute(u_int32_t dst_ip, u_int32_t gw_ip);

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
int del_kroute(u_int32_t dst_ip, u_int32_t gw_ip);

#endif
