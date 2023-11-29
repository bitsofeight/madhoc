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
 *        Resends a queued RREQ if retries is lower than RREQ_RETRIES
 *	  otherwise it throws the package away and returns error
 *
 *	Internal procedures:
 *	
 *	External procedures:
 *        rreq_timeout
 *        hello_resend
 *
 ********************************
 *
 Extendend RCS Info: $Id: to_rreq.c,v 1.5 2000/04/18 08:12:41 root Exp root $
 *
 */

#ifndef TO_RREQ
#define TO_RREQ

#include "timer.h"
#include "RT.h"
#include "rreq_list.h"

/* A structure for packets in the data field */
struct rreq_tdata
{
  struct rreq *rd; /* Pointer to a RREQ packet */
  int retries;     /* Number of times resent */
  int ttl;         /* The last sent TTL */
  u_int32_t dst_ip;/* The destination to send the RREQ to */
};

/* 
 *   rreq_timeout
 *
 *   Description: 
 *     Resends the RREQ in the input argument while keeping
 *     track of nr of times resent. If maximum times the 
 *     packet is thrown away and an error is returned by the
 *     function.
 *
 *   Arguments:
 *     struct rreq_tdata *rd - The data and the RREQ package that will be 
 *                             resent
 *
 *   Return:
 *     int - Returns -1 on error or maximum retries otherwise 0
 */
int rreq_timeout(struct rreq_tdata *rd);

/*  
 *   hello_resend
 *
 *   Description: 
 *     Resends the HELLO message and puts it back in the timer queue.
 *
 *   Arguments:
 *     struct rreq_tdata *rd - The data and the HELLO package that will be
 *                             resent.
 *
 *   Return: None
 */
void hello_resend(struct rreq_tdata *rd);

#endif

