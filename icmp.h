/* LibICMP - A simple API for sending and receiving ICMP datagrams.
 *
 * Copyright (c) 2000       Tim Lawless <tim.lawless@usm.edu>
 * Copyright (c) 2011-2022  Joachim Wiberg <troglobit@gmail.com>
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

#ifndef ICMP_ICMP_H_
#define ICMP_ICMP_H_

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

struct libicmp {
	int             sd;		/* File Descriptor for the ICMP Socket */
	int             gai_code;	/* Error code from getaddrinfo() */
	uint8_t         ttl;		/* IP unicast/multicast TTL */
	uint16_t        id;		/* The ICMP_ECHO ID */
	uint16_t        seqno;		/* Sequence # */
	char           *host;		/* Original hostname to ping. */
	struct timeval  tv;		/* Packet send time */
	int             triptime;	/* Round trip time */
};

struct libicmp *icmp_open   (char *host, uint16_t id, uint8_t ttl);
int             icmp_bind   (struct libicmp *obj, char *addr);
int             icmp_send   (struct libicmp *obj, uint8_t type,              char *payload, size_t len);
int             icmp_recv   (struct libicmp *obj, uint8_t type, int timeout, char *payload, size_t len);
int             icmp_ping   (struct libicmp *obj,                            char *payload, size_t len);
int             icmp_close  (struct libicmp *obj);

int             icmp_resolve(struct libicmp *obj, struct addrinfo **ai);
char           *icmp_ntoa   (struct libicmp *obj, char *buf, size_t len);

int             icmp_err    (struct libicmp *obj);
const char     *icmp_errstr (struct libicmp *obj);

#endif /* ICMP_ICMP_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
