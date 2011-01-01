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

#ifndef LIBICMP_H_
#define LIBICMP_H_

#include <netinet/ip_icmp.h>

#define MAX_DGRAM_SIZE 65502
#define BUFSIZE        65536

extern int      silent;

struct icmp_socket {
    int             socketfd;	/* File Descriptor for the ICMP Socket */
    int             pseudoport;	/* The ICMP_ECHO ID, refered to as the
				 * Sequence # */
    int             host;
};

typedef struct icmp_socket icmp_socket_t;

struct icmp_dgram {
    unsigned short  size;	/* Size at head of dgram */
    char            buf[MAX_DGRAM_SIZE];
};

typedef struct icmp_dgram icmp_dgram_t;

icmp_socket_t  *icmp_socket_open(unsigned long host,
				 unsigned short pseudoport);
icmp_dgram_t   *icmp_dgram_build(void *msg, int size);
icmp_dgram_t   *icmp_dgram_recv(icmp_socket_t * icmp_socket, u_int8_t type);
int             icmp_dgram_send(icmp_socket_t * icmp_socket, u_int8_t type,
				icmp_dgram_t * icmp_dgram);
int             icmp_socket_close(icmp_socket_t * icmp_socket);

#endif				/* LIBICMP_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 4
 * End:
 */
