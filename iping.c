/* iping - a very small and free ping utility to showcase libICMP
 *
 * Copyright (c) 2011-2016  Joachim Nilsson <troglobit@gmail.com>
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

#include <err.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "icmp.h"

int main(int argc, char *argv[])
{
	int   len, result = 0;
	char *host, addr[INET_ADDRSTRLEN];
	char  message[] = "iping: r u there?";
	struct libicmp *obj;

	if (argc < 2) {
		fprintf(stderr, "usage: ping <hostname>\n");
		return 1;
	}

	host = argv[1];
	obj = icmp_open(host, 0x1337, 0);
	if (!obj)
		err(1, "Failed opening ICMP socket");

	if (!icmp_ntoa(obj, addr, sizeof(addr))) {
		warnx("%s: %s", host, icmp_errstr(obj) ?: "<nil>");
		goto exit;
	}

	printf("PING %s (%s)\n", host, addr);
	while (1) {
		len = icmp_ping(obj, message, sizeof(message));
		if (len <= 0) {
			warnx("%s: %s", host, icmp_errstr(obj) ?: "<nil>");
			result = 1;
			goto exit;
		}

		printf("%d bytes from %s (%s): icmp_req=%d ttl=%d time=%d.%d ms\n",
		       len, host, addr, obj->seqno, obj->ttl, obj->triptime / 10, obj->triptime % 10);
		sleep(1);
	}

exit:
	icmp_close(obj);

	return result;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
