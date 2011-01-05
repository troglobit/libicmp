/* iping - a very small and free ping utility to showcase libicmp
 *
 * Copyright (c) 2011  Joachim Nilsson <troglobit@vmlinux.org>
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

#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libicmp.h"

#define ERROR(fmt, args...) fprintf(stderr, "%s: " fmt, __progname, ##args)
extern char *__progname;

char *resolve(char *host, char *buf, size_t len)
{
    char *result;
    struct addrinfo *addr;

    if (icmp_resolve(host, &addr))
	return NULL;

    result = icmp_ntoa(addr, buf, len);
    freeaddrinfo(addr);

    return result;
}

int main(int argc, char *argv[])
{
    size_t len;
    char *host, *addr;
    char buf[80];
    char message[] = "iping: r u there?";
    libicmp_t *isock;

    if (argc < 2) {
	fprintf(stderr, "usage: ping <hostname>\n");
	return 1;
    }

    host = argv[1];
    addr = resolve(host, buf, sizeof(buf));
    if (!addr) {
	ERROR("%s for %s\n", strerror (errno), host);
    }

    isock = icmp_open(host, 0x1337, 0);
    if (!isock) {
	ERROR("Failed to open ICMP socket: %s\n", strerror (errno));
	return 0;
    }

    printf("PING %s (%s)\n", host, addr);
    while (1) {
	len = icmp_ping(isock, message, sizeof(message));
	if (!len)
	    ERROR("%s for %s\n", strerror (errno), host);
	else
	    printf("%zd bytes from %s (%s): icmp_req=%d ttl=%d time=%ld.%ld ms\n",
		   len, host, addr, isock->seqno, isock->ttl, isock->triptime / 10, isock->triptime % 10);
	sleep(1);
    }

    icmp_close(isock);

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
