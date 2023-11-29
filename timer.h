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
 *  	  Defines a priority queue with timer interrupt via SIGALRM.
 *	  When a events occurs it writes an address to a queue entry to a
 *        pipe which can then be read and used.
 *	  When a entry has been used it has to be freed by the top function 
 *
 *	Internal procedures:
 *
 *	External procedures:
 *	  pq_new
 *	  pq_updatetimer
 *	  pq_signal
 *	  pq_readpipe
 *	  pq_insert
 *	  pq_getfirst
 *	  pq_getfirstofid
 *        pq_getfirstofidflags
 *	  pq_deleteent
 *	  pq_deleteidflags
 *	  pq_unqueuefirstent
 *        pq_unqueueidflags
 *	  pq_deletefirstent
 *	  pq_getfirstdue
 *	  pq_getfirstdueofid
 *	  pq_print
 *
 ********************************
 *
 * Extendend RCS Info: $Id: timer.h,v 1.7 2000/05/09 12:20:16 root Exp root $
 *
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "utils.h"


/* How much the currtime can diverge from set time to match (in ms)*/
#define TIME_DIV 100

/* Flags definitions */
#define PQ_PACKET_RREQ 1
#define PQ_PACKET_RREP 2
#define PQ_PACKET_RERR 3
#define PQ_PACKET_HELLO 4
#define PQ_FLAGS_ALL 255


/* prio queue entry */
struct prioqent
{
  u_int64_t tv;  /* Time the event should happend in ms since 1 jan 1970 */
  void *data;    /* Data stored in the entry */
  u_int32_t id;  /* An id used to match one or a group of entrys */
  unsigned char flags;  /* Flag which represents what's in the dataportion */
  struct prioqent *pqe;  /* A pointer to the next entry in the list */
};

/* The prio queue */
struct prioq
{
  struct prioqent *pqe;  /* Pointer to the first entry */
  int tpipe[2];          /* The pipe used to communicate with top program */
};


/* 
 *   pq_new
 *
 *   Description: 
 *     Initialize the prioqueue and set the
 *     function which will handel the timer
 *     interrupt.
 *
 *   Arguments: None
 *
 *   Return: 
 *     A file descriptor on which addresses to a 
 *     struct prioqent can be read (with pq_readpipe) 
 *     when an queued event has expired.
 *     On error -1 is returned.
 */
int pq_new();

/* 
 *   pq_updatetimer
 *
 *   Description: 
 *     Update the timer to the value of the event which
 *     is most recent. 
 *
 *   Arguments: None
 *
 *   Return: None
 */
void pq_updatetimer();

/* 
 *   pq_signal
 *
 *   Description: 
 *     Is run when the timer signals that an event has happend.
 *     It gets the entrys from the queue that is due and writes
 *     their addresses to the pipe.
 *
 *   Arguments: 
 *     int signal - The number of the signal that caused it to be called.
 *
 *   Return: None
 */
void pq_signal(int signal);

/* 
 *   pq_readpipe
 *
 *   Description: 
 *     Reads an address to a prioq entry from the pipe.
 *     Caution - If the function is called and the
 *		 pipe is empty the function WILL block.
 *
 *   Arguments: None
 * 
 *  Return: 
 *    struct prioqent* - This is the pointer to the prio queue entry that has
 *                       timed out.
 *		         This is the pointer which shall be freed when one is
 *		         done with it.
*/
struct prioqent *pq_readpipe();

/* 
 *   pq_insert
 *
 *   Description: 
 *     Inserts an entry into the prioqueue.
 *
 *   Arguments: 
 *     u_int64_t msec  - The time in milliseconds (msec from 1 jan 1970) of 
 *                       which the event shall occur.
 *     void *data  - Pointer to the data that shall be stored.
 *     u_int32_t id  - A number to identyfy the stored data (like an 
 *                       ipnumber or something)
 *     unsigned char flags  - A flag that decribes what is stored in the
 *                            data pointer 
 *
 *   Return: 
 *     int   - On error returns -1 else 0
 */
int  pq_insert(u_int64_t msec,void *data,u_int32_t id,unsigned char flags);

/* 
 *   pq_getfirst
 *
 *   Description: 
 *     Returns a pointer to the first entry in the queue.
 *
 *   Arguments: None
 *
 *   Return: 
 *     struct prioqent* - A ponter to the first entry in the queue.
 *                        Can be NULL if the queue is empty.
 */
struct prioqent *pq_getfirst();

/* 
 *   pq_getfirstofid
 *
 *   Description: 
 *     Returns a pointer to the first entry in the queue
 *     that matches the id.
 *
 *   Arguments: 
 *     u_int32_t id - The id to be matched by the entry
 * 
 *   Return: 
 *     struct prioqent* - A ponter to the first entry in the queue that
 *                        matches the supplied id.
 *                        Can be NULL if the queue is empty or there is no
 *		          entry with that id.
 */
struct prioqent *pq_getfirstofid(u_int32_t id);

/* 
 *   pq_getfirstofidflags
 *
 *   Description: 
 *     Returns a pointer to the first entry in the queue
 *     that matches the id.
 *
 *   Arguments: 
 *     u_int32_t id        - The id to be matched by the entry
 *     unsigned char flags - The flags to be matched
 *
 *   Return: 
 *     struct prioqent* - A ponter to the first entry in the queue that
 *                        matches the supplied id and the supplied flags.
 *                        Can be NULL if the queue is empty or there is no
 *		          entry with that id.
 */
struct prioqent *pq_getfirstofidflags(u_int32_t id,unsigned char flags);

/* 
 *   pq_deleteent
 *
 *   Description: 
 *     Deletes the entry from the queue that is pointed to 
 *     by the argument. The entry is freed.
 *
 *   Arguments: 
 *     struct prioqent *pqed - A pointer to the entry that shall be deleted.
 *
 *   Return: None
 */
void pq_deleteent(struct prioqent *pqed);

/* 
 *   pq_deleteidflags
 *
 *   Description: 
 *     Deletes all entrys from the queue that matches the id and
 *     flag given as arguments. The entrys are freed.
 *
 *   Arguments: 
 *     u_int32_t id        - The id that shall be matched. 
 *     unsigned char flags - The flags that shall be matched 
 *                           (Can be PQ_FLAGS_ALLL for all flags). 
 *
 *   Return: None
 *   Note: Possible memory leak here, deleting entrys but not the data 
 *         Possible bug deleting entrys but they could be set as timers
 */
void pq_deleteidflags(u_int32_t id,unsigned char flags);

/* 
 *   pq_unqueuefirstent
 *
 *   Description: 
 *     Remove but no delete the first entry in the queue. (not freed)
 *
 *   Arguments: None
 *
 *   Return: None
 */
void pq_unqueuefirstent();

/* 
 *   pq_unqueueidflags
 *
 *   Description: 
 *     Unqueues the first entry in the queue
 *     that matches the id and flags.
 *
 *   Arguments: 
 *     u_int32_t id        - The id to be matched by the entry
 *     unsigned char flags - The flags to be matched
 *
 *   Return: Void 
 */
void pq_unqueueidflags(u_int32_t id,unsigned char flags);

/* 
 *   pq_deletefirstent
 *
 *   Description: 
 *     Deletes the first entry in the queue. 
 *
 *   Arguments: None
 *
 *   Return: None
 */
void pq_deletefirstent();

/* 
 *   pq_getfirstdue
 *
 *   Description: 
 *     Returns the first entry in the queue that has a time 
 *     that is lower than the argument ie. the first element
 *     if the argument is greater than its tv value.
 *
 *   Arguments: 
 *     u_int64_t tv - the time that the elment to be returned shall be
 *                    lower than. This is not quite true due to the real
 *                    time timer vs. the timeslice for the process the
 *                    value shound be in an interval +/- TIME_DIV to the
 *                    time tv given.
 *
 *   Return: 
 *     struct prioqent* - Pointer to the first entry if the time tv was
 *                        right else NULL is returned.
 */
struct prioqent *pq_getfirstdue(u_int64_t tv);

/* 
 *   pq_getfirstdueofid
 *
 *   Description: 
 *     Same as pq_getfirstdue exept it is the the first due that 
 *     matches the id given as argument.
 *
 *   Arguments: 
 *     u_int64_t tv - Same as pq_getfirstdue.
 *     u_int32_t id - The id that shall be matched.
 *
 *   Return: 
 *     struct prioqent* - Pointer to the first entry that matches 
 *                        id and if the time tv was right else
 *                        NULL is returned.
 */
struct prioqent *pq_getfirstdueofid(u_int64_t tv,u_int32_t id);

/* 
 *   pq_print
 *
 *   Description: 
 *     Prints the prio queue.
 *
 *   Arguments: None
 * 
 *  Return: None
 */
void pq_print();

#endif
