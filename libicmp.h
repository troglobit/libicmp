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

#define MAX_DGRAM_LEN  65502
#define BUFSIZE        65536

extern int      silent;

struct libicmp {
    int             sd;         /* File Descriptor for the ICMP Socket */
    int             gai_code;   /* Error code from getaddrinfo() */
    u_int8_t        ttl;        /* IP unicast/multicast TTL */
    u_int16_t       id;         /* The ICMP_ECHO ID */
    u_int16_t       seqno;      /* Sequence # */
    char           *host;	   /* Original hostname to ping. */
    struct timeval  tv;         /* Packet send time */
    long            triptime;   /* Round trip time */
};

typedef struct libicmp libicmp_t;

int         icmp_resolve(char *host, struct addrinfo **addr);
char       *icmp_ntoa(struct addrinfo *addr, char *buf, size_t len);

libicmp_t  *icmp_open(char *host, unsigned short id, unsigned ttl);
size_t      icmp_recv(libicmp_t *isock, char *buf, u_int8_t type, int timeout);
int         icmp_send(libicmp_t *isock, u_int8_t type, char *payload, size_t len);
int         icmp_ping(libicmp_t *isock, char *payload, size_t len);
int         icmp_close(libicmp_t *isock);

#endif /* LIBICMP_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 4
 * End:
 */
