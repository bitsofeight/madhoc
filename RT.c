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
 *        This module handles the functionality of the AODV
 *        routing table. The functionality mainly consists of insertion, 
 *        deletion and returning of list elements such as routing table
 *        entries and precursors.
 *
 *	Internal procedures: 
 *        find_precursor
 *	
 *	External procedures: 
 *        init_rt
 *        get_first_entry
 *        insert_entry
 *        get_entry
 *        delete_entry
 *        add_precursor
 *        delete_precursor
 *        delete_precursors_from_all
 *        clear_precursors
 *        print_rt
 ********************************
 *
 * Extendend RCS Info: $Id: RT.c,v 1.11 2000/05/10 18:33:57 root Exp root $
 *
 */

#include "RT.h"

struct rt_entry_list  *rt;

/* Declaration of internal procedures */
struct precursor* find_precursor(struct artentry* tmp_artentry,
				 u_int32_t tmp_ip);

/*
 * init_rt
 *
 * Description: 
 *   Initializes the main routing table by creating a 
 *   head for the list.
 *
 * Arguments: void
 *
 * Return: void
 */
int
init_rt()
{
  if((rt = malloc(sizeof(struct rt_entry_list))) != NULL)
    {
      rt->ishead = 1;
      rt->next = rt;
      rt->prev = rt;
      
      if ((krt = init_rtsocket()) < 0) /* for the krt socket */
	return -1; /* Unable to create socket */

      return 0;
    }

  return -1;
}


/*
 * get_first_entry
 *
 * Description: 
 *   Returns the first entry in the routing table
 *
 * Arguments: void
 *
 * Return: 
 *  struct rt_entry* - Pointer to the first list entry of the routing table.
 */
struct rt_entry_list*
get_first_entry()
{
  return rt->next;
}


/*
 * insert_entry
 *
 * Description: 
 *   Allocates memory for a new routing table entry and the
 *   rt_entry_list-element pointing to it. The list-element
 *   is inserted in the routing table.
 *
 * Arguments: void
 *
 * Returns: 
 *   struct artentry* - A pointer to the new routing table entry. 
 */
struct artentry*
insert_entry()
{
  struct rt_entry_list *tmp_rt_entry_list;
  struct precursor *tmp_precursor;
  struct artentry *tmp_artentry;

  /* Allocate memory for new entry */
  if((tmp_precursor = malloc(sizeof(struct precursor))) == NULL)
    return NULL;

  if((tmp_artentry = (struct artentry*) 
      malloc(sizeof(struct artentry))) == NULL)
    return NULL;

  if((tmp_rt_entry_list = (struct rt_entry_list*)
      malloc(sizeof(struct rt_entry_list))) == NULL)
    return NULL;

  /* Create precursor head */
  tmp_precursor->ishead = 1;
  tmp_precursor->next = tmp_precursor;
  tmp_precursor->prev = tmp_precursor;
  tmp_precursor->ip = 0;

  tmp_artentry->precursors = tmp_precursor;
  tmp_rt_entry_list->entry = tmp_artentry;
  tmp_rt_entry_list->ishead = 0;

  tmp_rt_entry_list->prev = rt;
  tmp_rt_entry_list->next = rt->next;
  tmp_rt_entry_list->next->prev = tmp_rt_entry_list;
  tmp_rt_entry_list->prev->next = tmp_rt_entry_list;
  
  return tmp_rt_entry_list->entry;
}


/*
 * getentry
 *
 * Description: 
 *   Returns a pointer to the routing table entry having
 *   the argument ip-address as destination.
 *
 * Arguments: 
 *   u_int32_t tmp_ip - IP address for the destination
 *
 * Returns: 
 *   struct artentry* - Pointer to the routing table entry, if existing. 
 *                      Otherwise NULL.
 */
struct artentry*
getentry(u_int32_t tmp_ip)
{
  struct rt_entry_list *tmp_rt_entry_list;
  struct in_addr tmp_in_addr;

  for(tmp_rt_entry_list = rt->next;
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)
    {
      if(tmp_rt_entry_list->entry->dst_ip == tmp_ip)
	{
	  tmp_in_addr.s_addr = tmp_ip;
	  return tmp_rt_entry_list->entry;
	}
    }

  return NULL;
}


/*
 * delete_entry
 *
 * Description: 
 *   Deletes the routing table entry having the argument ip
 *   address as destination.
 *
 * Arguments: 
 *   u_int32_t tmp_ip - Destination IP address
 *
 * Returns: void
 */
void
delete_entry(u_int32_t tmp_ip)
{
  struct rt_entry_list *tmp_rt_entry_list;

  for(tmp_rt_entry_list = rt->next;
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)
    {
      if(tmp_rt_entry_list->entry->dst_ip == tmp_ip)
	{
	  del_kroute(tmp_rt_entry_list->entry->dst_ip,
		    tmp_rt_entry_list->entry->nxt_hop);
	  
	  tmp_rt_entry_list->prev->next = tmp_rt_entry_list->next;
	  tmp_rt_entry_list->next->prev = tmp_rt_entry_list->prev;
	  clear_precursors(tmp_rt_entry_list->entry);
	  free(tmp_rt_entry_list->entry->precursors);
	  free(tmp_rt_entry_list->entry);
	  free(tmp_rt_entry_list);
	  break;
	}
    }
}

/*
 * krt_cleanup
 *
 * Description: 
 *   Removes all routes that exist in the AODV routing table
 *   from the kernel's routing table.
 *
 * Arguments: void
 *
 * Returns: void
 */
void
krt_cleanup()
{
  struct rt_entry_list *tmp_rt_entry_list;

  for(tmp_rt_entry_list = get_first_entry();
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)

    {
      if(del_kroute(tmp_rt_entry_list->entry->dst_ip,
		   tmp_rt_entry_list->entry->nxt_hop) == 0)
	printf("Kernel route to: %s removed.\n",
	       inet_ntoa(*((struct in_addr *)&
			   (tmp_rt_entry_list->entry->dst_ip)))); 
    }
}


/*
 * add_precursor
 *
 * Description: 
 *   Allocates memory for a new precursor and inserts it in
 *   the precursor list for the destination given by the argument.
 *
 * Arguments: 
 *   struct artentry *tmp_artentry - Routing table entry for the destination
 *   u_intr32_t tmp_ip - IP address for the precursor to insert
 *
 * Returns: 
 *   int - 0 if precursor is successfully created
 *        -1 otherwise.
 */
int
add_precursor(struct artentry *tmp_artentry, u_int32_t tmp_ip)
{
  struct precursor *tmp_precursor;
  
  if(find_precursor(tmp_artentry, tmp_ip) == NULL)
    {
      if((tmp_precursor = malloc(sizeof(struct precursor))) == NULL)
	return -1;

      tmp_precursor->next = tmp_artentry->precursors->next;
      tmp_precursor->prev = tmp_artentry->precursors;
      tmp_precursor->prev->next = tmp_precursor;
      tmp_precursor->next->prev = tmp_precursor;
      tmp_precursor->ip = tmp_ip;
      tmp_precursor->ishead = 0;
      tmp_artentry->precursors->next = tmp_precursor;
    }
  
  return 0;
}


/*
 * delete_precursor
 *
 * Description: 
 *   Deletes a precursor from a routing table entry.
 *
 * Arguments: 
 *   struct artentry *tmp_artentry - The routing table entry for the 
 *                                   destination
 *   u_int32_t tmp_ip - IP address for the precursor to delete.
 *
 * Returns: void
 */
void
delete_precursor(struct artentry *tmp_artentry, u_int32_t tmp_ip)
{
  struct precursor *tmp_precursor;

  if((tmp_precursor = find_precursor(tmp_artentry, tmp_ip)) != NULL)
    {
      tmp_precursor->prev->next = tmp_precursor->next;
      tmp_precursor->next->prev = tmp_precursor->prev;
      free(tmp_precursor);
    }
}


/*
 * delete_precursor_from_all
 *
 * Description: 
 *   Deletes a precursor from all entries it appears in in the
 *   routing table.
 *
 * Arguments: 
 *   u_int32_t tmp_ip - IP address for the precursor to delete.
 *
 * Returns: void
 */
void
delete_precursor_from_all(u_int32_t tmp_ip)
{
  struct rt_entry_list *tmp_rt_entry_list;
  struct precursor *tmp_precursor;

  for(tmp_rt_entry_list = rt->next;
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)
    {
      if((tmp_precursor = find_precursor(tmp_rt_entry_list->entry,
					 tmp_ip)) != NULL)
	{
	  tmp_precursor->prev->next = tmp_precursor->next;
	  tmp_precursor->next->prev = tmp_precursor->prev;
	  free(tmp_precursor);
	}
    }
}


/* 
 * find_precursor
 *
 * Description: 
 *   Returns a pointer to the precursor in the routing table
 *   entry given by the arguments.
 *
 * Arguments: 
 *   struct artentry *tmp_artentry - Routing table entry to search in
 *   u_int32_t tmp_ip - IP address to search for
 *
 * Returns: 
 *   struct precursor* - Pointer to the precursor, if it exists. 
 *                       Otherwise NULL.
 */
struct precursor*
find_precursor(struct artentry *tmp_artentry, u_int32_t tmp_ip)
{
  struct precursor *tmp_precursor;

  for(tmp_precursor = tmp_artentry->precursors->next;
      tmp_precursor->ishead == 0;
      tmp_precursor = tmp_precursor->next)
    {
      if(tmp_precursor->ip == tmp_ip)
	  return tmp_precursor;

    }

  return NULL;
}


/* 
 * clear_precursors
 *
 * Description: 
 *   Removes the whole precursor list from a given routing
 *   table entry.
 *
 * Arguments: 
 *   struct artentry *tmp_artentry - The routing table entry in which 
 *                                   to remove the precursors.
 *
 * Returns: void
 */
void
clear_precursors(struct artentry *tmp_artentry)
{
  struct precursor *tmp_precursor;

  for(tmp_precursor = tmp_artentry->precursors->next;
      tmp_precursor->ishead == 0;
      tmp_precursor = tmp_precursor->next)
    {
      tmp_precursor->prev->next = tmp_precursor->next;
      tmp_precursor->next->prev = tmp_precursor->prev;
      free(tmp_precursor);
    }
}


/* 
 * print_rt
 *
 * Description: 
 *   Prints the routing table
 *
 * Arguments: void
 *
 * Returns: void
 */
void
print_rt()
{
  struct in_addr tmp_in_addr1, tmp_in_addr2, tmp_in_addr3;
  struct rt_entry_list *tmp_rt_entry_list;
  struct precursor *tmp_precursor;
  struct artentry *tmp_artentry;
  char ip_str1[24];
  char ip_str2[24];
  char ip_str3[24];

  printf("Dst ip\t\tDst seq\tBcst id\tHop cnt\tLast hop cnt"
	 "\tNext hop\tPrecursors\tLifetime\n");

  for(tmp_rt_entry_list = rt->next;
      tmp_rt_entry_list->ishead != 1;
      tmp_rt_entry_list = tmp_rt_entry_list->next)
    {
      tmp_artentry = tmp_rt_entry_list->entry;
      tmp_precursor = tmp_artentry->precursors->next;
      tmp_in_addr1.s_addr = tmp_artentry->dst_ip;
      tmp_in_addr2.s_addr = tmp_artentry->nxt_hop;
      tmp_in_addr3.s_addr = tmp_precursor->ip;
      strcpy(ip_str1, inet_ntoa(tmp_in_addr1));
      strcpy(ip_str2, inet_ntoa(tmp_in_addr2));
      strcpy(ip_str3, inet_ntoa(tmp_in_addr3));
      
      printf("%s\t", ip_str1);
      if(strlen(ip_str1) < 8)
	printf("\t");

      printf("%u\t%u\t%d\t%d\t\t%s\t%s\t",
	     tmp_artentry->dst_seq,
	     tmp_artentry->broadcast_id,
	     tmp_artentry->hop_cnt,
	     tmp_artentry->lst_hop_cnt,
	     ip_str2,
	     ip_str3);
      if(strlen(ip_str3) < 8)
	printf("\t");
      
      printf("%Lu\n", tmp_artentry->lifetime);
      for(tmp_precursor = tmp_precursor->next;
	  tmp_precursor->ishead != 1;
	  tmp_precursor = tmp_precursor->next)
	{
	  tmp_in_addr3.s_addr = tmp_precursor->ip;
	  printf("\t\t\t\t\t\t\t\t\t");
	  strcpy(ip_str3, inet_ntoa(tmp_in_addr3));
	  printf("%s\n", ip_str3);
	}
    }
}
