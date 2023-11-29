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
 *          Receive Route Request.
 *          Receives the route requests from other nodes and
 *          acts dependingly of the route table. If the request
 *          is for it self, or it has an active route, it sends
 *          back a reply. Otherwise it forwards to requst to a
 *          new broadcast. 
 *
 *      Internal procedures:
 *      
 *      External procedures:
 *          rec_rreq(struct info, struct rreq)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: rreq.h,v 1.5 2000/04/17 12:07:59 root Exp root $
 *
 */

#ifndef RREQ_H
#define RREQ_H

#include <stdio.h>
#include <stdlib.h>

#include "aodv.h"
#include "RT.h"
#include "rt_entry.h"
#include "info.h"
#include "utils.h"
#include "gen_rrep.h"
#include "rreq_list.h"
#include "update_reverse.h"

/*
 * rec_rreq
 *
 * Description:  
 *   Receive the route request. Looks up the routing table
 *   to determine if there is an active route to the 
 *   destination. An active route is either to the node
 *   it self, or a destination that there already exists
 *   a route that isn't too old.
 *   It then calls the right functions, either a route reply
 *   or retransmits the route request in broadcast.
 *
 * Arguments:    
 *   struct info *inf - The information structure containing information on
 *                      IP-level such as IP-addresses.
 *   struct rreq *in_rreq - Structure that contains the informations about
 *                          the route request.
 *
 * Return: 
 *   int - 0 - if succesful
 *        -1 - if memory couldn't be allocated
 */
int rec_rreq(struct info *inf, struct rreq *in_rreq);

#endif
