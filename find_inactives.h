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
 *        Detects routes that have expired and marks them
 *        as expired in the routing table. When an expired route is not
 *        renewed and times out the route is deleted from the routing table.
 *        find_inactives is called every HELLO_INTERVAL. If the expired node
 *        is a neighbour link_break is called to inform other neighbours.
 *
 *      Internal procedures: 
 *      
 *      External procedures: 
 *        find_inactives()
 *
 ********************************
 *
 *      Extendend RCS Info: $Id$
 *
 */

#ifndef FIND_INACTIVES_H
#define FIND_INACTIVES_H

#include <sys/types.h>

#include"rerr.h"
#include"RT.h"

/*
 * find_inactives
 *
 * Descritpion:
 *   Finds inactive entries 
 *
 * Arguments: Void
 *
 * Return: Void
 */
void find_inactives();

#endif





