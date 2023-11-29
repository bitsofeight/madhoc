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
 *          Receives Route Reply
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          rec_rrep(struct info, struct rrep)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: rrep.h,v 1.6 2000/04/17 13:26:51 root Exp root $
 *
 */

#ifndef RREP_H
#define RREP_H

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
#include "timer.h"
#include "krtable.h"

/*
 * rec_rrep
 *
 * Description:  
 *   Receives and handles RREP
 *
 * Arguments:    
 *   struct info *my_info - Pointer to the received information structure
 *   struct rrep *my_rrep - Pointer to the received route reply structure
 *
 * Return:
 *   int - 0 
 */
int rec_rrep(struct info *my_info, struct rrep *my_rrep);

#endif
