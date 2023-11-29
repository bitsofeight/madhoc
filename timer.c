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
 *	  Defines a priority queue with timer interrupt via SIGALRM.
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
 *	  pq_getfirstofidflags
 *	  pq_deleteent
 *	  pq_deleteidflags
 *	  pq_unqueuefirstent
 *	  pq_unqueueidflags
 *	  pq_deletefirstent
 *	  pq_getfirstdue
 *	  pq_getfirstdueofid
 *	  pq_print
 *
 ********************************
 *
 * Extendend RCS Info: $Id: timer.c,v 1.6 2000/05/09 12:20:51 root Exp root $
 *
 */

#include "timer.h"

/* 
 *   pq
 *
 *   Description: 
 *     A global pointer to the one and only prioqueue
 *
 *   Arguments: N/A
 *
 *   Return: N/A
*/
struct prioq *pq;

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

int
pq_new()
{

  if (pq != NULL || 
      (pq = (struct prioq*)malloc(sizeof(struct prioq))) == NULL)
    return -1;

  /* Get the pipe */
  if (pipe(pq->tpipe) < 0)
    return -1;
  
  /* Register signal handler */
  signal(SIGALRM,(void*)pq_signal);
  
  return pq->tpipe[0];
}


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
void
pq_updatetimer()
{
  struct itimerval ovalue;
  struct itimerval value;
  u_int64_t currtime;
  u_int64_t tv;

  /* Init timer values to zero */
  (ovalue.it_interval).tv_sec = 0;
  (ovalue.it_interval).tv_usec = 0;
  
  (value.it_interval).tv_sec = 0;
  (value.it_interval).tv_usec = 0;


  if (pq->pqe == NULL)
    {
      /* No event to set timer for */
      (value.it_value).tv_sec = 0;
      (value.it_value).tv_usec = 0;
    }
  else 
    {
      /*  Get the first time value */
      tv = (pq->pqe)->tv;
      
      currtime = getcurrtime();
      
      if (tv <= currtime)
	{
	  /* If the event has allready happend, set the timeout to 
	     1 microsecond :-) */
	  (value.it_value).tv_sec = 0;
	  (value.it_value).tv_usec = 1;
	}
      else 
	{
	  /* Set the timer to the actual seconds / microseconds from now */
	  (value.it_value).tv_sec =  (tv - currtime) / 1000;
	  (value.it_value).tv_usec = ((tv - currtime) % 1000) * 1000;
	}
      
    }
  
  /* Set the timer (in real time) */
  if (setitimer(ITIMER_REAL,&value,&ovalue) < 0)
    /* Error */
    return;
  
}


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
void
pq_signal(int signal)
{
  struct prioqent *pqe;
  u_int64_t currtime;
  
  /* Get the first due entry in the queue */
  currtime = getcurrtime();
  pqe = pq_getfirstdue(currtime);
  
  /* While there is still events that has timed out */
  while (pqe != NULL)
    {
      /* Write the address of the entry to the pipe */
      if(write (pq->tpipe[1], &pqe, sizeof(struct prioqent*)) < 0)
	/* Error writing to the pipe */
	return;

      /* Dequeue the entry so that it will not happened again */
      pq_unqueuefirstent();
      
      /* Get new time and check for more timedout entrys */
      currtime = getcurrtime();
      
      pqe = pq_getfirstdue(currtime);
    }
  
}


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
struct prioqent*
pq_readpipe()
{
  struct prioqent **pqe;
  struct prioqent *tpqe;
  
  /* Get temporary memory to write into */
  if ((pqe = (struct prioqent**)malloc(sizeof(struct prioqent*))) < 0)
    /* Failed to allocate memory */
    return NULL;
  
  /* Read from the pipe (BLOCKING CALL)*/
  if (read(pq->tpipe[0], pqe, sizeof(struct prioqent*)) < 0)
    /* Failed to read from pipe */
    return NULL;
  
  /* Copy address pointed to */
  tpqe = *pqe;
  /* free temporary space */
  free (pqe);
  
  return tpqe;
}

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
int 
pq_insert(u_int64_t msec,void *data,u_int32_t id,unsigned char flags)
{
  struct prioqent *next_pqe;
  struct prioqent *prev_pqe;
  struct prioqent *pqe;
  
  /* get memory */
  if ((pqe = (struct prioqent*)malloc(sizeof(struct prioqent))) == NULL)
    /* Failed to allocate memory */
    return -1;

  /* copy data */
  pqe->tv = msec;
  pqe->data = data;
  pqe->id = id;
  pqe->flags = flags;

  /* find a place for the new entry */
  
  /* is the queue empty or we are the first element */
  if (pq->pqe == NULL || (pq->pqe)->tv > pqe->tv)
    {
      pqe->pqe = pq->pqe;
      pq->pqe = pqe;
    }
  else 
    {
      /* we are further down in the list */
      next_pqe = (pq->pqe)->pqe;
      prev_pqe = pq->pqe;
      
      /* Go down while we are not at the end and all elements are due 
	 sooner than us */
      while (next_pqe != NULL && 
	     (prev_pqe->tv < pqe->tv && (next_pqe->tv) < pqe->tv) )
	{
	  prev_pqe = next_pqe;
	  next_pqe = next_pqe->pqe;
	}
      /* Reached the end of the list */
      if (next_pqe == NULL)
	{
	  prev_pqe->pqe = pqe;
	  pqe->pqe = NULL;
	}
      /* Found a place to insert it into */
      else 
	{
	  pqe->pqe = next_pqe;
	  prev_pqe->pqe = pqe;
	}
      
    }
  /* Update the timer to reflect the new situation */
  pq_updatetimer();
  
  return 0;
}


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
struct prioqent * 
pq_getfirst()
{
  return pq->pqe;
}


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
struct prioqent * 
pq_getfirstofid(u_int32_t id)
{
  struct prioqent *pqe;
  
  /* Get first entry */
  pqe = pq->pqe;
  
  while (pqe != NULL && pqe->id != id)
    pqe=pqe->pqe;
  
  return pqe;
}

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
struct prioqent * 
pq_getfirstofidflags(u_int32_t id, unsigned char flags)
{
  struct prioqent *pqe;
  
  /* Get first entry */
  pqe = pq->pqe;
  
  while (pqe != NULL && !(pqe->id == id && pqe->flags == flags))
    pqe = pqe->pqe;
  
  return pqe;
}

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
void
pq_unqueueidflags(u_int32_t id, unsigned char flags)
{
  struct prioqent *pqe_next;
  struct prioqent *pqe_prev;
  
  if (pq->pqe != NULL) 
    {
      /* is the first element */
      while (pq->pqe != NULL && (pq->pqe)->id == id && 
	     ((pq->pqe)->flags == flags || flags == PQ_FLAGS_ALL))
	{
	  /* Loop while its the first element */
	  pqe_next = pq->pqe;
	  pq->pqe = (pq->pqe)->pqe;
	}
      
      if (pq->pqe != NULL) 
	{
	  /* The queue is not empty when we are done with the first entrys */
	  pqe_next = (pq->pqe)->pqe;
	  pqe_prev = pq->pqe;
	  
	  while(pqe_next != NULL)
	    {
	      /* Loop trough the remaining entrys */
	      if (pqe_next->id == id  && 
		  ((pqe_next->flags == flags || flags == PQ_FLAGS_ALL)))
		{
		  pqe_prev->pqe = pqe_next->pqe;
		  pqe_next = pqe_prev->pqe;
		}
	      else
		{
		  pqe_prev = pqe_next;
		  pqe_next = pqe_next->pqe;
		} 
	    }
	}
    }
  /* Change the timer to reflect the new chenges */
  pq_updatetimer();
  
}

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
void
pq_deleteent(struct prioqent *pqed)
{
  struct prioqent *pqe;
  
  /* Is first element */
  if (pq->pqe == pqed)
    {
      pq->pqe = pqed->pqe;
      free(pqed);
    }
  else 
    {
      /* Not the first element in the list */
      pqe = pq->pqe;
      while (pqe->pqe != NULL && pqe->pqe != pqed)
	pqe = pqe->pqe;
      
      if (pqe != NULL)
	{
	  pqe->pqe = pqed->pqe;
	  free(pqed);
	}
    }
  
  pq_updatetimer();
}


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
void
pq_deleteidflags(u_int32_t id, unsigned char flags)
{
  struct prioqent *pqe_next;
  struct prioqent *pqe_prev;
  
  if (pq->pqe != NULL) 
    {
      /* is the first element */
      while (pq->pqe != NULL && (pq->pqe)->id == id && 
	     ((pq->pqe)->flags == flags || flags == PQ_FLAGS_ALL))
	{
	  /* Loop while its the first element */
	  pqe_next = pq->pqe;
	  pq->pqe = (pq->pqe)->pqe;
	  free(pqe_next);
	}
      
      if (pq->pqe != NULL) 
	{
	  /* The queue is not empty when we are done with the first entrys */
	  pqe_next = (pq->pqe)->pqe;
	  pqe_prev = pq->pqe;
	  
	  while(pqe_next != NULL)
	    {
	      /* Loop trough the remaining entrys */
	      if (pqe_next->id == id  && 
		  ((pqe_next->flags == flags || flags == PQ_FLAGS_ALL)))
		{
		  pqe_prev->pqe = pqe_next->pqe;
		  free(pqe_next);
		  pqe_next = pqe_prev->pqe;
		}
	      else
		{
		  pqe_prev = pqe_next;
		  pqe_next = pqe_next->pqe;
		}
	    }
	}
    }
  /* Change the timer to reflect the new chenges */
  pq_updatetimer();
}


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
void
pq_unqueuefirstent()
{
  struct prioqent *pqe;

  if (pq->pqe != NULL)
    {
      pqe = pq->pqe;
      pq->pqe = pqe->pqe;
    }

  pq_updatetimer();
}

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
void
pq_deletefirstent()
{
  struct prioqent *pqe;

  if (pq->pqe != NULL)
    {
      pqe = pq->pqe;
      pq->pqe = pqe->pqe;
      free(pqe);
    }
  
  pq_updatetimer();
}

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
 *                    time timer vs. the timeslice for the process the value
 *                    shound be in an interval +/- TIME_DIV to the time
 *                    tv given.
 *
 *   Return: 
 *     struct prioqent* - Pointer to the first entry if the time tv was
 *                        right else NULL is returned.
*/
struct prioqent *
pq_getfirstdue(u_int64_t tv)
{
  struct prioqent *pqe;
  
  if ((pqe = pq_getfirst(pq)) != NULL)
    {
      /* If pqe's time is in teh interval */
      if ((pqe->tv) < tv + TIME_DIV || (pqe->tv) < tv - TIME_DIV)
	return pqe;
      
    }
  
  return NULL;
}

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
struct prioqent *
pq_getfirstdueofid(u_int64_t tv, u_int32_t id)
{
  struct prioqent *pqe;
  
  if ((pqe = pq_getfirstofid(id)) != NULL)
    {
      if ((pqe->tv) < tv + TIME_DIV || (pqe->tv) < tv - TIME_DIV)
	return pqe;
    }
  
  return NULL;
}


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
void
pq_print()
{
  struct prioqent *pqe;

  pqe = pq->pqe;
  while (pqe != NULL)
    {
      printf("sec/msec: %lu/%lu id:%lu\n", (unsigned long)(pqe->tv) / 1000,
	     (unsigned long)(pqe->tv) % 1000, (unsigned long)pqe->id);
      pqe = pqe->pqe;
    }
  
}
