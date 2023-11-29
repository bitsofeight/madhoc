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
 *          Updates the routing table when receiving RREQ
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          update_reverse(struct info *my_info, struct rreq *my_rreq)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: update_reverse.h,v 1.3 2000/04/18 09:02:28 root Exp root $
 *
 */

#ifndef UPDATE_REVERSE_H
#define UPDATE_REVERSE_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h> 

#include "RT.h"
#include "rt_entry.h"
#include "info.h"
#include "aodv.h"
#include "utils.h"
#include "krtable.h"


/* 
 *  update_reverse
 * 
 *  Description:  
 *    Updates the RT
 *
 *  Arguments:    
 *    struct info *my_info - Contains the IP informtion about the RREQ
 *    struct rreq *my_rreq -  Contains the RRQ
 *
 *  Return:
 *    int - 0
*/
int update_reverse(struct info *my_info, struct rreq *my_rreq);

#endif

