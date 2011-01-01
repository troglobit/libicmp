
/* Libicmp 0.1a
 * Tim.Lawless@usm.edu
 * 
 * Libicmp is intended to provide a simple interface for sending and reciving
 * icmp datagrams. Beyond the obvious diffrence of using ICMP, applications
 * Using this interface would behave much in the same way an UDP application
 * behaves. 
 *
 * The documentation for this version is non-existant. I know how
 * Works. I should write some, but I figure if your playing with
 * this you probably are skilled enough to understand what I'm doing
 * from the two included "test" applications. 
 *
 * Feel free to take, use, improve. Use any lisence you desire.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include "libicmp.h"

u_short
cksum (u_short * buf, int nwords)
{
  unsigned long sum;

  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}


/* If your opening up a socket for listening, set both paramaters to 0 */
ICMP_SOCKET *
open_icmp_socket (unsigned long int host, unsigned short int psudoport)
{
  ICMP_SOCKET *isock;

  isock = malloc (sizeof (ICMP_SOCKET));
  bzero (isock, sizeof (ICMP_SOCKET));

  if (host || psudoport)
    {
      isock->host = host;
      isock->psudoport = psudoport;
    }

  if ((isock->socketfd = socket (AF_INET, SOCK_RAW, 1)) < 0)
    {
	perror ("libicmpnet::open_icmp_socket");
      return NULL;
    }

  return isock;
}

ICMP_DGRAM *build_icmp_dgram (void *msg, int size)
{
  ICMP_DGRAM *dgram;

  if (size > MAX_DGRAM_SIZE)
    return NULL;

  if (!(dgram = malloc (sizeof(ICMP_DGRAM))))
    return NULL;

  bzero (dgram, sizeof (ICMP_DGRAM));

  dgram->size = size;

  if (!(memcpy (dgram->buf, msg, size)))
    {
      free (dgram);
      return NULL;
    }
  return dgram;
}


ICMP_DGRAM *
get_icmp_dgram (ICMP_SOCKET * icmp_socket)
{

  char buffer[BUFSIZE];
  struct iphdr *ip = (struct iphdr *) buffer;
  struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof (struct iphdr));
  ICMP_DGRAM * icmp_dgram;
  ICMP_DGRAM * dgram_overlay = (ICMP_DGRAM *) (buffer + sizeof (struct iphdr) +
				  sizeof(struct icmphdr));

  int i, checksum;

  while (1)
    {

      if (!icmp_socket
	  || ((i = read (icmp_socket->socketfd, buffer, BUFSIZE)) < 0))
	{
	  return NULL;
	}

      if (ip->protocol != 1 || icmp->type != 8 ||
	   icmp->un.echo.id != icmp_socket->psudoport)
	continue;

      checksum = icmp->checksum;
      icmp->checksum = 0;
	

      if (checksum !=  cksum ((u_short *) icmp,
			  sizeof (struct icmphdr) + dgram_overlay->size +
			  sizeof (unsigned short int)))
					  return NULL;
	  icmp_dgram = malloc (sizeof (ICMP_DGRAM));
      
	  if (!icmp_dgram)
			  return NULL;
      bzero (icmp_dgram, sizeof (ICMP_DGRAM));
      /* First, Extract our Size */

      memcpy (icmp_dgram, dgram_overlay, dgram_overlay->size + 
					  sizeof(unsigned short int));
	  printf("Icmp Dgram, size: %d, Message: %s\n",icmp_dgram->size,
					  icmp_dgram->buf);
      return icmp_dgram;


    }		

  /* Of While. We probably could rop alot out of the loop, but if it */
  /* does not hit the continue, then we will exit the function..i    */
  /* somehow. */

}


int
send_icmp_dgram (ICMP_SOCKET * icmp_socket, ICMP_DGRAM * icmp_dgram)
{

  char buffer[BUFSIZE - sizeof (struct iphdr)];
  struct icmphdr *icmp = (struct icmphdr *) buffer;
  struct sockaddr_in sockaddr;
  int i;

  bzero(buffer,BUFSIZE - sizeof (struct iphdr));

  memcpy ((buffer + sizeof (struct icmphdr)), icmp_dgram,
	  icmp_dgram->size + sizeof (unsigned short int));

  icmp->type = 8;
  icmp->un.echo.id = icmp_socket->psudoport; 
  icmp->checksum = cksum ((u_short *) icmp, sizeof(struct icmphdr) + 
				  icmp_dgram->size + sizeof (unsigned short int)); 
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = icmp_socket->host;

  if (
      (i =
       sendto (icmp_socket->socketfd, buffer,
	       sizeof (struct icmphdr) + icmp_dgram->size + 
		   sizeof (unsigned short int), 0,
	       (struct sockaddr *) &sockaddr, sizeof (sockaddr))) < 0)
    return 0;

  return i;
}



int
close_icmp_socket (ICMP_SOCKET * icmp_socket)
{
  if (!icmp_socket)
    return errno;

  if (icmp_socket->socketfd)
    close (icmp_socket->socketfd);

  free (icmp_socket);

  return 0;
}
