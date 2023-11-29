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
 *          id  and is only valid some amount of time. The requests are
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
 * Extendend RCS Info: $Id: rreq_list.c,v 1.7 2000/04/17 12:09:02 root Exp root $
 *
 */

#include "rreq_list.h"

struct rreq_entry    *rreq_header;  /* Global pointer to the list's head */
int   list_is_init = 0; /* Set to 1 when the list has been initialized */

/* Pre-declaration of internal function */
int check_lifetime(struct rreq_entry *in_entry);

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
int
init_rreq_list()
{
  /* Check if the list already has been initialzed */
  if (!list_is_init)
    {
      /* Allocate memory for the head of the list */
      if ((rreq_header = (struct rreq_entry*)
	   malloc(sizeof(struct rreq_entry))) == NULL)
	/* Failed to allocate memory for list-header */
	return -1;
    }
  
  /* Set to variables in the head to zero values */
  rreq_header->src_ip = 0;
  rreq_header->broadcast_id = 0;
  rreq_header->lifetime = 0;
  rreq_header->prev = NULL;
  rreq_header->next = NULL;

  /* The list has now been initialzed */
  list_is_init = 1;

  return 0;
}

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
struct rreq_entry*
find_rreq(u_int32_t src_ip, u_int32_t bc_id)
{
  struct rreq_entry  *tmp_entry;  /* Working entry in the RREQ list */
  struct rreq_entry  *prev_entry; /* Entry previous to the working entry */
  struct rreq_entry  *tmp_ptr = NULL; /* Pointer to an entry in RREQ list */

  tmp_entry = rreq_header; /* Start at the header */
  prev_entry = NULL;

  /* Check each entry in the list if it's a desired entry, starting with 
     the header*/
  do {
    /* If the working entry isn't NULL, save a pointer to the next entry */
    if (tmp_entry != NULL)
      tmp_ptr = tmp_entry->next;

    /* Check if the current entry is still valid */
    if (check_lifetime(tmp_entry) == 0)
      {	
	/* Entry was valid */

	/* Check if this is the desired entry */
	if (src_ip == tmp_entry->src_ip && bc_id == tmp_entry->broadcast_id)
	  return tmp_entry; /* Right entry found, return it back */
	
	/* Not the right entry, step to the next */
	prev_entry = tmp_entry;
	tmp_entry = tmp_entry->next;
      }
    else
      /* Entry wasn't valid, step to the next entry */
      if (tmp_entry != rreq_header)
	tmp_entry = tmp_ptr;
      else
	tmp_entry = NULL;
    
  } while (tmp_entry != NULL);/* Continue until the entire list is checked */
  
  return NULL;  /* No entry was found */
}

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
int
add_rreq(u_int32_t ip, u_int32_t id, u_int64_t lt)
{
  struct rreq_entry  *tmp_entry; /* Pointer to the working entry */
  struct rreq_entry  *new_entry; /* The new entry to be added */

  tmp_entry = rreq_header;

  /* Check if the header is empty. If so, add the info in the header */
  if (tmp_entry->src_ip == 0) 
    {
      tmp_entry->src_ip = ip;
      tmp_entry->broadcast_id = id;
      tmp_entry->lifetime = lt;
    }
  else
    {
      /* The header wasn't empty, find the last entry */
      while (tmp_entry->next != NULL)
	tmp_entry = tmp_entry->next;

      /* Allocate memory for the new entry */
      if ((new_entry = (struct rreq_entry*)
	   malloc(sizeof(struct rreq_entry))) == NULL)
	/* Failed to allocate memory for new Route Request */
	return -1;

      /* Fill in the information in the new entry */
      new_entry->src_ip = ip;
      new_entry->broadcast_id = id;
      new_entry->lifetime = lt;
      new_entry->prev = tmp_entry;
      new_entry->next = NULL;
      
      /* Put the new entry in the list */
      tmp_entry->next = new_entry;

    }

  return 0;
}

/*
 * check_lifetime
 *
 * Description: 
 *   Checks if the entry in the list is still valid.
 *   If not valid, the entry is removed
 *
 * Arguments: 
 *   struct rreq_entry *in_entry - The RREQ entry to be checked
 * 
 * Return:
 *   int - 0 if the entry was valid
 *        -1 if the entry was old
 */
int
check_lifetime(struct rreq_entry *in_entry)
{
  u_int64_t  curr_time = getcurrtime(); /* Current time */

  /* Check if the entry is valid */
  if (curr_time > in_entry->lifetime)
    { 
      /* The entry was old - remove it */

      /* Set the previous and next pointers */
      if (in_entry->next != NULL)
	(in_entry->next)->prev = in_entry->prev;

      if (in_entry->prev != NULL)
	(in_entry->prev)->next = in_entry->next;
      
      /* If the list only consists of header, reset the list
	 else free the memory of the entry */
      if (in_entry == rreq_header)
	/* Call init_rreq_list to reinitialize the list */
	init_rreq_list();
      else
	free (in_entry);

      return -1;
    }

  return 0;
}
