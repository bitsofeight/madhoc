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
 *        Handels a user interaction menu system to perform 
 *        differnet functions in the program.
 *        You can:
 *        Print the routing table
 *        Add a route to the routing table
 *        Generate a RREQ
 *        Generate a RERR (link break)
 *
 *	Internal procedures:
 *        strcnt
 *        strrep
 *
 *	External procedures:
 *        io_read
 *        io_parse
 *
 ********************************
 *
 * Extendend RCS Info: $Id: uio.c,v 1.9 2000/04/18 08:28:25 root Exp root $
 *
 */

#include "uio.h"


/* 
 *   strcnt
 *
 *   Description: 
 *     Counts the number of occurences of 
 *     a character in a string.
 *
 *   Arguments:
 *     char *cs - A pointer to a string to search.
 *     char c   - The character to serach for.
 *
 *   Return:
 *     int - The number of occurences of c in cs
*/
int 
strcnt(char *cs, char c)
{
  int count = 0;
  char *p = cs;

  while ((p = strchr(p, c)) != NULL)
    {
      count++;
      p++;
    }
  
  return count;
}

/* 
 *   strrep
 *
 *   Description: 
 *     Replaces every occurance of character
 *     c1 with character c2 in cs and returns
 *     the number replaced.
 *
 *   Arguments:
 *     char *cs - A pointer to a string to searched and replaced.
 *     char c1  - The character to serach for.
 *     char c2  - The character to repace c1 with.
 *
 *   Return:
 *     int - The number of repaced characters.
*/
int 
strrep(char *cs, char c1, char c2)
{
  int count = 0;
  char *p = cs;

  while ((p = strchr(p, c1)) != NULL)
    {
      count++;
      *p = c2;
      p++;
    }
  
  return count;
}

/* 
 *   io_read
 *
 *   Description: 
 *     Prints a menu and reads input form user.
 *     The read input can then be used with io_parse.
 *
 *   Arguments: None.
 *
 *   Return: 
 *     char * - A pointer to the data read from the user.
 *              Has to be freed by the reciving fucnction.
*/
char *
io_read()
{
  char *buff;
  
  printf("\ngen_rreq:xxx.xxx.xxx.xxx\nprint_rt\nadd_rt:dst_ip:" 
	 "dst_seq:broadcast_id:hop_cnt:lst_hop_cnt:nxt_hop:lifetime:" 
	 "rt_flags\nlink_break:xxx.xxx.xxx.xxxn\nCommand: ");
  
  if ((buff = malloc(MAXLEN*sizeof(char))) == NULL)
    /* Failed to allocate memory */
    return NULL;
  
  fscanf(stdin, "%s", buff);
  /*  fgets(buff,MAXLEN,stdin);*/
  
  return buff;
}


/* 
 *   io_parse
 *
 *   Description: 
 *     Parse the given in data and take actions based
 *     upon the input.
 *     Can do:
 *	Print the routing table
 *	Add a route to the routing table
 *	Generate a RREQ
 *	Generate a RERR (link break)
 *
 *   Arguments: 
 *     char *io_string - The sting to be parsed
 *     struct info *is - Data about the environmens such as my ip.
 *
 *   Return: 
 *     int - Returns -1 on error otherwise 0
*/
int
io_parse(char *io_string, struct info *is)
{
  struct artentry *rte;
  char ipbuff[MAXLEN];
  int retval = -1;
  char *io_p;

  /* Check if the input is invalid */
  if (is == NULL || io_string == NULL)
    {
      if (io_string != NULL)
	/* Free the io data */
	free(io_string);

      return -1;
    }
  
  /* Is a generate route request ? */
  if (strncmp(io_string,IO_GEN_RREQ_STR,strlen(IO_GEN_RREQ_STR)) == 0)
    {
      /* Check if correct io_string */
      if (strlen(io_string) > strlen(IO_GEN_RREQ_STR) + ADDRLEN)
	retval = -1;
      
      if (sscanf(io_string + strlen(IO_GEN_RREQ_STR) + 1, "%s", ipbuff) != 1)
	retval = -1;
      
      is->ip_pkt_dst_ip = inet_addr(ipbuff);
      is->ip_pkt_ttl = 0;
      printf("%s\n", ipbuff);
      gen_rreq(is);

      return 0;
    }
  /* Is a print routing table ? */
  else if (strncmp(io_string, IO_PRINT_RT_STR, strlen(IO_PRINT_RT_STR)) == 0)
    print_rt();
  
  /* Is an add to routing table ? */
  else if (strncmp(io_string, IO_ADD_RT_STR, strlen(IO_ADD_RT_STR)) == 0)
    {
      if (strcnt(io_string, ':') == 8)
	{
	  
	  strrep(io_string, ':', '\0');
	  
	  /* Fill in the struct */
	  rte = insert_entry();
	  
	  io_p = strchr(io_string, '\0');
	  rte->dst_ip = inet_addr(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->dst_seq = atol(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->broadcast_id = atol(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->hop_cnt= atol(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->lst_hop_cnt = atol(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->nxt_hop = inet_addr(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->lifetime = getcurrtime() + atol(++io_p);
	  
	  io_p = strchr(io_p, '\0');
	  rte->rt_flags = atol(++io_p);
	  /* add route to kernel's rtable */
	  if(add_kroute(rte->dst_ip, rte->nxt_hop))
	    {
	      /* add_kroute failed in uio, ignore and continue */
	    }
	}
      /* is it a RERR/link_break? */
    }
  else if (strncmp(io_string, IO_GEN_RERR_STR, strlen(IO_GEN_RERR_STR)) == 0)
    {
      if (sscanf(io_string + strlen(IO_GEN_RERR_STR) + 1, "%s", ipbuff) != 1)
	retval = -1;
      
      link_break(is,inet_addr(ipbuff));
      return 0;
    }
    
  free(io_string);

  return retval;
}











