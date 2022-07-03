/* iping - a very small and free ping utility to showcase libICMP
 *
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

#include <config.h>
#include <arpa/inet.h>
#include <err.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "icmp.h"
#define PATTERN "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "

char *ident = PACKAGE_NAME;

static int usage(int code)
{
	printf("Usage: %s [OPTIONS] HOST\n"
	       "\n"
	       "Options:\n"
	       "  -h          This help text\n"
	       "  -s SIZE     Generate payload of SIZE bytes\n"
	       "  -S SOURCE   Source IP to use in ICMP datagram, from interface\n"
	       "  -t TTL      Set IP time to live (hops)\n"
	       "  -V          Verbose operation, dump payload, etc.\n"
	       "  -v          Show version information\n"
	       "\n"
	       "Bug report address: %-40s\n"
	       "Project homepage: %s\n\n", ident, PACKAGE_BUGREPORT, PACKAGE_URL);

	return code;
}

static char *progname(char *arg0)
{
       char *nm;

       nm = strrchr(arg0, '/');
       if (nm)
	       nm++;
       else
	       nm = arg0;

       return nm;
}

static char *generator(char *buf, size_t buflen)
{
        size_t num, len;
        static size_t pos = 0;
        const char pattern[] = PATTERN;

        len = 72;
	if (len > buflen)
		len = buflen;
        if (pos + len > sizeof(pattern)) {
                num = sizeof(pattern) - pos;
                len -= num;
        } else {
                num = 72;
		if (num > buflen)
			num = buflen;
		len = 0;
        }

        strncpy(&buf[0], &pattern[pos], num--);
        if (len++)
                strncpy(&buf[num], pattern, len);

        if (++pos >= sizeof(pattern) - 1)
                pos = 0;

        return buf;
}

int main(int argc, char *argv[])
{
	int   len, result = 0, verbose = 0;
	char *source_ip = NULL, ch;
	char *host, addr[INET_ADDRSTRLEN];
	size_t payloadsz = 0;
	uint8_t ttl = 0;
	struct libicmp *obj;

	ident = progname(argv[0]);
	while ((ch = getopt(argc, argv, "hs:S:t:vV")) != EOF) {
		switch (ch) {
		case 's':
			payloadsz = (size_t)atoi(optarg);
			break;

		case 'S':
			source_ip = optarg;
			break;

		case 't':
			ttl = atoi(optarg);
			break;

		case 'v':
			printf("v%s\n", PACKAGE_VERSION);
			return 0;

		case 'V':
			verbose = 1;
			break;

		case '?':
		case 'h':
			return usage(0);

		default:
			return usage(1);
		}
	}

	if (optind == argc)
		return usage(1);

	host = argv[optind];
	obj = icmp_open(host, 0x1337, ttl);
	if (!obj)
		err(1, "Failed opening ICMP socket");

	if (!icmp_ntoa(obj, addr, sizeof(addr))) {
		warnx("%s: %s", host, icmp_errstr(obj) ?: "<nil>");
		goto exit;
	}

	if (source_ip && icmp_bind(obj, source_ip)) {
		warnx("%s: %s", source_ip, icmp_errstr(obj) ?: "<nil>");
		goto exit;
	}

	printf("PING %s (%s)\n", host, addr);
	while (1) {
		char buf[BUFSIZ];
		size_t buflen = 0;

		if (payloadsz > 0) {
			buflen = sizeof(buf) < payloadsz ? sizeof(buf) : payloadsz;
			generator(buf, buflen);
		}

		if (icmp_send(obj, ICMP_ECHO, buf, buflen))
			goto error;

		len = icmp_recv(obj, ICMP_ECHOREPLY, 5000, buf, sizeof(buf));
		if (len < 0) {
		error:
			warnx("%s: %s", host, icmp_errstr(obj) ?: "<nil>");
			result = 1;
			goto exit;
		}

		printf("PING reply from %s (%s): icmp_req=%d ttl=%d time=%d.%d ms\n",
		       host, addr, obj->seqno, obj->ttl, obj->triptime / 10, obj->triptime % 10);
		if (verbose && len > 0) {
			buf[len] = 0;
			printf("\tPayload: %s\n", buf);
		}
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
