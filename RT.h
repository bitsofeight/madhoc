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

#ifndef RT_H
#define RT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "rt_entry_list.h"
#include "krtable.h"

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
struct rt_entry_list* get_first_entry();

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
struct artentry* getentry(u_int32_t);

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
void delete_entry(u_int32_t);

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
void krt_cleanup();


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
struct artentry* insert_entry();

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
int add_precursor(struct artentry* tmp_artentry, u_int32_t tmp_ip);

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
void delete_precursor(struct artentry* tmp_artentry, u_int32_t tmp_ip);

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
void clear_precursors(struct artentry* tmp_artentry);

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
void delete_precursor_from_all(u_int32_t tmp_ip);

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
int init_rt();

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
void print_rt();

#endif
