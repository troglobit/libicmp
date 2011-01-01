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

#include "libicmp.h"
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  HOSTNAME  "127.0.0.1"
#define  PSEUDOPORT 54321

unsigned long
resolve(char *p)
{
    struct hostent *h;
    unsigned long int rv;

    if ((h = gethostbyname(p)) == NULL) {
	perror("gethostbyname");
	exit(1);
    }

    if (h != NULL)
	memcpy(&rv, h->h_addr, h->h_length);
    else
	rv = inet_addr(p);

    return rv;
}

int
main(void)
{
    icmp_socket_t  *isock;
    icmp_dgram_t   *idgram;
    char            message[] = "Hello World.";
    int             i;

    isock = icmp_socket_open(resolve(HOSTNAME), PSEUDOPORT);
    if (!isock) {
	printf("Failed to Open Socket. \n");
	exit(0);
    }

    idgram = icmp_dgram_build(message, sizeof(message));
    i = icmp_dgram_send(isock, ICMP_ECHO, idgram);
    printf("Sent %d Bytes.\n", i);

    icmp_socket_close(isock);

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
