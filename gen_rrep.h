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
 *          Generates a Route Reply
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          gen_rrep(struct info, struct rreq)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: gen_rrep.h,v 1.3 2000/04/17 13:04:40 root Exp root $
 *
 */

#ifndef GEN_RREP_H
#define GEN_RREP_H

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

/*
 * gen_rrep
 *
 * Description:  
 *   Generates a RREP
 *
 * Arguments:    
 *   struct info *my_info - The information on IP level
 *   struct rreq *my_rreq - The RREQ that needs a RREP
 *
 * Return:       
 *   int - 0 (- if succesful)
 */
int
gen_rrep(struct info *my_info, struct rreq *my_rreq);

#endif






