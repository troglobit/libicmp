
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
#include "libicmp.h"
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define  HOSTNAME 127.0.0.1
#define  PSUDOPORT 54321

unsigned long int resolve(char *p)
{
  struct   hostent *h;
  unsigned long int rv;

  if ((h = gethostbyname(p)) == NULL) {
    perror("gethostbyname");
    exit(-1);
  }

  if(h != NULL) memcpy(&rv, h->h_addr, h->h_length);
  else rv = inet_addr(p);

  return rv;
}

int main ( void )
{

  ICMP_SOCKET *isock;
  ICMP_DGRAM *idgram;
  char message[] = "Hello World.";
  int i;

  if (!(isock = open_icmp_socket(resolve("127.0.0.1"), 54321)))
	    {  printf("Failed to Open Socket. \n"); exit(0); }



 
  idgram =  build_icmp_dgram(message,sizeof(message));

  i = send_icmp_dgram(isock,idgram); 

  printf("Sent %d Bytes.\n",i);

  close_icmp_socket(isock);   
  return 0 ;

}

