
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

#ifndef LIBICMP_H_
#define LIBICMP_H_

#define MAX_DGRAM_SIZE 65502
#define BUFSIZE 65536

extern int silent;

struct icmp_socket {
       int socketfd; /* File Descriptor for the ICMP Socket */
       int psudoport; /* The ICMP_ECHO ID, refered to as the Sequence # */
	   int host;
       }; 

typedef struct icmp_socket ICMP_SOCKET;


struct icmp_dgram {
       unsigned short int  size; 	/* Size at head of dgram */
       char buf[MAX_DGRAM_SIZE];
       };
       

typedef struct icmp_dgram ICMP_DGRAM;

ICMP_DGRAM * build_icmp_dgram( void * msg , int size ); 
ICMP_SOCKET * open_icmp_socket (unsigned long int host, 
				unsigned short int psudoport ); 		
ICMP_DGRAM * get_icmp_dgram( ICMP_SOCKET * icmp_socket); 
int send_icmp_dgram(ICMP_SOCKET * icmp_socket, ICMP_DGRAM * icmp_dgram ); 		
int close_icmp_socket ( ICMP_SOCKET * icmp_socket ); 							

#endif  /* LIBICMP_H_ */
