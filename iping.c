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

#include "libicmp.h"

unsigned long resolve(char *p)
{
    struct hostent *h;
    unsigned long int rv;

    if ((h = gethostbyname(p)) == NULL) {
	fprintf(stderr, "Failed looking up IP address for %s: %s\n", p, hstrerror (h_errno));
	exit(1);
    }

    if (h != NULL)
	memcpy(&rv, h->h_addr, h->h_length);
    else
	rv = inet_addr(p);

    return rv;
}


int main(int argc, char *argv[])
{
    int i;
    char message[] = "iping: r u there?";
    icmp_socket_t *isock;
    icmp_dgram_t *idgram;

    if (argc < 2) {
	fprintf(stderr, "usage: ping <hostname>\n");
	return 1;
    }

    isock = icmp_socket_open(resolve(argv[1]), 54321);
    if (!isock) {
	fprintf(stderr, "Failed to open ICMP socket: %s\n", strerror (errno));
	return 0;
    }

    idgram = icmp_dgram_build(message, sizeof(message));
    i = icmp_dgram_send(isock, ICMP_ECHO, idgram);
    idgram = icmp_dgram_recv(isock, ICMP_ECHOREPLY);
    if (!idgram) {
	printf("Something is Wrong with icmp_dgram_recv.\n");
	perror("Humm.");
	exit(0);
    }
    printf("Dgram Size: %d, Dgram Message: %s\n",idgram->size,idgram->buf);

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
