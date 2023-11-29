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
 *          Handles the route request list. A certain request has it's own
 *          id and is only valid some amount of time. The requests are
 *          entered, looked up and removed here.
 * 
 *      Internal procedures:
 *          check_lifetime(struct rreq_entry)
 *      
 *      External procedures:
 *          init_rreq_list()
 *          find_rreq(u_int32_t, u_int32_t)
 *          add_rreq(u_int32_t, u_int32_t, u_int64_t)
 *
 ********************************
 *
 * Extendend RCS Info: $Id: rreq_list.h,v 1.3 2000/04/17 12:08:46 root Exp root $
 *
 */
#ifndef RREQ_LIST_H
#define RREQ_LIST_H

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#include "utils.h"

struct rreq_entry
{
  u_int32_t          src_ip;
  u_int32_t          broadcast_id;
  u_int64_t          lifetime;
  struct rreq_entry *prev;
  struct rreq_entry *next;
};

/*
 * init_rreq_list
 *
 * Description:  
 *   Creates and initializes the list for route requests
 *
 * Arguments: void
 *   
 * Returns: 
 *   int - 0 if succesfull
 *        -1 if failed to allocate memory for list header
 */
int init_rreq_list();

/*
 * find_rreq
 *
 * Description:  
 *   Searches for an entry in the route request list.
 *
 * Arguments:
 *   u_int32_t scr_ip - The IP address of the sender of the RREQ
 *   u_int32_t bc_id - The broadcast ID of the RREQ
 *
 * Return:
 *   struct rreq_entry* - Pointer to the found entry.
 *                        If no entry found, NULL
 */
struct rreq_entry *find_rreq(u_int32_t src_ip, u_int32_t bc_id);

/*
 * add_rreq
 *
 * Description:  
 *   Adds a new entry to the route request list
 *
 * Arguments:
 *   u_int32_t ip - IP address of the sender of the RREQ
 *   u_in32_t  id - Broadcast ID of the RREQ
 *   u_int65_t lt - Lifetime of the RREQ
 *
 * Return:
 *   int- 0 if succesful
 *       -1 if failed to allocate memory for a new entry
 */
int add_rreq(u_int32_t src_ip, u_int32_t bd_id, u_int64_t lifetime);

#endif
