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
 *        Handels a user interaction menu system to perform 
 *        differnet functions in the program.
 *        You can:
 *        Print the routing table
 *        Add a route to the routing table
 *        Generate a RREQ
 *        Generate a RERR (link break)
 *
 *	Internal procedures:
 *        strcnt
 *        strrep
 *
 *	External procedures:
 *        io_read
 *        io_parse
 *
 ********************************
 *
 * Extendend RCS Info: $Id: uio.c,v 1.9 2000/04/18 08:28:25 root Exp root $
 *
 */

#ifndef UIO_H
#define UIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "aodv.h"
#include "info.h"
#include "gen_rreq.h"
#include "RT.h"
#include "rerr.h"


#define IO_FD 0
/* Constant strings to match in the menu commands */
#define IO_GEN_RREQ_STR  "gen_rreq"
#define IO_PRINT_RT_STR  "print_rt"
#define IO_ADD_RT_STR    "add_rt"
#define IO_GEN_RERR_STR  "link_break"

#define MAXLEN 255

/* The max length of an ip address */
#define ADDRLEN 4 * 3 + 3


/* 
 *   io_read
 *
 *   Description: 
 *     Prints a menu and reads input form user.
 *     The read input can then be used with io_parse.
 *
 *   Arguments: None.
 *
 *   Return: 
 *     char * - A pointer to the data read from the user.
 *              Has to be freed by the reciving fucnction.
*/
char *io_read();

/* 
 *   io_parse
 *
 *   Description: 
 *     Parse the given in data and take actions based
 *     upon the input.
 *     Can do:
 *	Print the routing table
 *	Add a route to the routing table
 *	Generate a RREQ
 *	Generate a RERR (link break)
 *
 *   Arguments: 
 *     char *io_string - The sting to be parsed
 *     struct info *is - Data about the environmens such as my ip.
 *
 *   Return: 
 *     int - Returns -1 on error otherwise 0
*/
int io_parse(char *io_string, struct info *is);

#endif
