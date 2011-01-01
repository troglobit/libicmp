/* Libicmp - A simple interface for sending and receiving ICMP datagrams.
 *
 * Copyright (c) 2000  Tim Lawless <tim.lawless@usm.edu>
 * Copyright (c) 2011  Joachim Nilsson <troglobit@vmlinux.org>
 *
 * Libicmp is intended to provide a simple interface for sending and reciving
 * icmp datagrams. Beyond the obvious diffrence of using ICMP, applications
 * Using this interface would behave much in the same way an UDP application
 * behaves. 
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

static u_short
cksum(u_short * buf, int nwords)
{
    unsigned long   sum;

    for (sum = 0; nwords > 0; nwords--)
	sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}


/*
 * If your opening up a socket for listening, set both paramaters to 0
 */
icmp_socket_t  *
icmp_socket_open(unsigned long int host, unsigned short int pseudoport)
{
    icmp_socket_t  *isock;

    isock = malloc(sizeof(icmp_socket_t));
    if (!isock)
	return NULL;

    bzero(isock, sizeof(icmp_socket_t));

    if (host || pseudoport) {
	isock->host = host;
	isock->pseudoport = pseudoport;
    }

    if ((isock->socketfd = socket(AF_INET, SOCK_RAW, 1)) < 0)
	return NULL;

    return isock;
}

icmp_dgram_t   *
icmp_dgram_build(void *msg, int size)
{
    icmp_dgram_t   *dgram;

    if (size > MAX_DGRAM_SIZE)
	return NULL;

    if (!(dgram = malloc(sizeof(icmp_dgram_t))))
	return NULL;

    bzero(dgram, sizeof(icmp_dgram_t));

    dgram->size = size;

    if (!(memcpy(dgram->buf, msg, size))) {
	free(dgram);
	return NULL;
    }
    return dgram;
}


icmp_dgram_t   *
icmp_dgram_recv(icmp_socket_t * icmp_socket, u_int8_t type)
{
    int             i,
                    checksum;
    char            buffer[BUFSIZE];
    struct iphdr   *ip = (struct iphdr *) buffer;
    struct icmphdr *icmp =
	(struct icmphdr *) (buffer + sizeof(struct iphdr));
    icmp_dgram_t   *icmp_dgram;
    icmp_dgram_t   *dgram_overlay;

    dgram_overlay =
	(icmp_dgram_t *) (buffer + sizeof(struct iphdr) +
			  sizeof(struct icmphdr));
    while (1) {
	if (!icmp_socket
	    || ((i = read(icmp_socket->socketfd, buffer, BUFSIZE)) < 0))
	    return NULL;

	if (ip->protocol != 1 || icmp->type != type ||
	    icmp->un.echo.id != htons(icmp_socket->pseudoport))
	    continue;

	checksum = icmp->checksum;
	icmp->checksum = 0;
	if (checksum != cksum((u_short *) icmp,
			      sizeof(struct icmphdr) +
			      dgram_overlay->size +
			      sizeof(unsigned short int)))
	    return NULL;

	icmp_dgram = malloc(sizeof(icmp_dgram_t));
	if (!icmp_dgram)
	    return NULL;
	bzero(icmp_dgram, sizeof(icmp_dgram_t));

	/*
	 * First, Extract our Size
	 */
	memcpy(icmp_dgram, dgram_overlay, dgram_overlay->size +
	       sizeof(unsigned short int));
	printf("Icmp Dgram, size: %d, Message: %s\n", icmp_dgram->size,
	       icmp_dgram->buf);

	return icmp_dgram;
    }
}


int
icmp_dgram_send(icmp_socket_t * icmp_socket, u_int8_t type,
		icmp_dgram_t * icmp_dgram)
{
    char            buffer[BUFSIZE - sizeof(struct iphdr)];
    struct icmphdr *icmp = (struct icmphdr *) buffer;
    struct sockaddr_in sockaddr;
    int             i;

    bzero(buffer, BUFSIZE - sizeof(struct iphdr));

    memcpy((buffer + sizeof(struct icmphdr)), icmp_dgram,
	   icmp_dgram->size + sizeof(unsigned short int));

    icmp->type = ICMP_ECHO;
    icmp->un.echo.id = htons(icmp_socket->pseudoport);
    icmp->checksum = cksum((u_short *) icmp, sizeof(struct icmphdr) +
			   icmp_dgram->size + sizeof(unsigned short int));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = icmp_socket->host;

    i = sendto(icmp_socket->socketfd, buffer,
	       sizeof(struct icmphdr) + icmp_dgram->size +
	       sizeof(unsigned short int), 0,
	       (struct sockaddr *) &sockaddr, sizeof(sockaddr));
    if (i < 0)
	return 0;

    return i;
}


int
icmp_socket_close(icmp_socket_t * icmp_socket)
{
    if (!icmp_socket)
	return errno;

    if (icmp_socket->socketfd)
	close(icmp_socket->socketfd);

    free(icmp_socket);

    return 0;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 4
 * End:
 */
