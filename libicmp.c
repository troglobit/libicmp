/* Libicmp - A simple interface for sending and receiving ICMP datagrams.
 *
 * Copyright (c) 2000       Tim Lawless <tim.lawless@usm.edu>
 * Copyright (c) 2011-2016  Joachim Nilsson <troglobit@gmail.com>
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

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <poll.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "libicmp.h"

static uint16_t in_cksum(uint16_t *buf, int nwords)
{
	uint32_t sum;

	for (sum = 0; nwords > 0; nwords--)
		sum += *buf++;

	sum  = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return ~sum;
}


/*
 * If you are opening up a socket for listening, set both paramaters to 0
 */
struct libicmp *icmp_open(char *host, uint16_t id, uint8_t ttl)
{
	struct libicmp  *obj;
	socklen_t        ttl_len;
	struct addrinfo *addr;

	/* Check that host exists first */
	if (icmp_resolve(host, &addr))
		return NULL;

	freeaddrinfo(addr);

	obj = malloc(sizeof(struct libicmp));
	if (!obj)
		return NULL;

	memset(obj, 0, sizeof(struct libicmp));

	obj->sd = socket(AF_INET, SOCK_RAW, 1);
	obj->id = id;
	obj->host = strdup(host);

	if (obj->sd < 0) {
		free(obj->host);
		free(obj);
		return NULL;
	}

	if (ttl) {
		ttl_len = sizeof(obj->ttl);
		setsockopt(obj->sd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
		setsockopt(obj->sd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
		getsockopt(obj->sd, IPPROTO_IP, IP_TTL, &obj->ttl, &ttl_len);
	}

	return obj;
}

int icmp_resolve(char *host, struct addrinfo **addr)
{
	int             code;
	struct addrinfo hints;

	res_init();		/* Reinitialize resolver every time to prevent cache misses due to
				 * NAME-->IP DNS changes, or outages in round robin DNS setups. */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = 0;
	hints.ai_protocol = 0;	/* Any protocol */

	code = getaddrinfo(host, NULL, &hints, addr);
	if (code) {
		errno = EINVAL;
		return code;
	}

	return 0;
}

char *icmp_ntoa(struct addrinfo *addr, char *buf, size_t len)
{
	/* NI_NUMERICHOST avoids DNS lookup. */
	if (getnameinfo(addr->ai_addr, addr->ai_addrlen, buf, len, NULL, 0, NI_NUMERICHOST))
		return NULL;

	return buf;
}

int icmp_send(struct libicmp *obj, uint8_t type, char *payload, size_t len)
{
	int              result;
	char             buffer[BUFSIZ];
	struct timeval   now;
	struct icmphdr  *icmp;
	struct addrinfo *addr;

	if (!obj) {
		errno = EINVAL;
		return -1;
	}

	obj->gai_code = icmp_resolve(obj->host, &addr);
	if (obj->gai_code)
		return -1;

	memset(buffer, 0, sizeof(buffer));
	gettimeofday(&now, NULL);

	/* ICMP Payload is our time now + any user defined payload */
	memcpy((buffer + sizeof(struct icmphdr)), &now, sizeof(struct timeval));
	memcpy((buffer + sizeof(struct icmphdr) + sizeof(struct timeval)), payload, len);

	icmp = (struct icmphdr *)buffer;
	icmp->type = type;
	icmp->un.echo.id = htons(obj->id);
	icmp->un.echo.sequence = htons(obj->seqno);
	icmp->checksum = in_cksum((u_short *) icmp, sizeof(struct icmphdr) + sizeof(struct timeval) + len);

	result = sendto(obj->sd, buffer, sizeof(struct icmphdr) + sizeof(struct timeval) + len, 0,
			addr->ai_addr, sizeof(struct sockaddr));
	freeaddrinfo(addr);
	if (result < 0)
		return -1;

	return 0;
}


int icmp_recv(struct libicmp *obj, uint8_t type, int timeout, char *payload, size_t len)
{
	int             i, checksum;
	char           *ptr, buf[BUFSIZ];
	struct iphdr   *ip;
	struct icmphdr *icmp;

	if (!obj) {
		errno = EINVAL;
		return -1;
	}

	ip   = (struct iphdr *)buf;
	icmp = (struct icmphdr *)(buf + sizeof(struct iphdr));
	ptr  = buf + sizeof(struct iphdr) + sizeof(struct icmphdr);

	while (1) {
		int           result = 0;
		struct pollfd pfd = { obj->sd, POLLIN | POLLPRI, 0 };

		result = poll(&pfd, 1, timeout);
		if (result <= 0) {
			if (result == 0)
				errno = ETIMEDOUT;

			return 0;
		}

		if (pfd.revents & (POLLIN | POLLPRI)) {
			size_t datalen;

			i = read(obj->sd, buf, BUFSIZ);
			if (i < 0)
				return -1;

			if (ip->protocol != 1 || icmp->type != type ||
			    icmp->un.echo.id != htons(obj->id) || icmp->un.echo.sequence != htons(obj->seqno))
				continue;

			checksum = icmp->checksum;
			icmp->checksum = 0;
			datalen = i - sizeof(struct iphdr);
			if (checksum != in_cksum((u_short *) icmp, datalen)) {
				errno = EIO;
				return -1;
			}

			memcpy(&obj->tv, ptr, sizeof(struct timeval));
			datalen -= sizeof(struct timeval);
			if (len < datalen)
				datalen = len;
			memcpy(payload, ptr + sizeof(struct timeval), datalen);

			return datalen;
		}
	}
}


int icmp_ping(struct libicmp *obj, char *payload, size_t len)
{
	char           buf[BUFSIZ];
	struct timeval now;

	obj->seqno++;
	if (icmp_send(obj, ICMP_ECHO, payload, len))
		return -1;

	len = icmp_recv(obj, ICMP_ECHOREPLY, 5000, buf, sizeof(buf));
	if (len <= 0)
		return -1;

	gettimeofday(&now, NULL);
	timersub(&now, &obj->tv, &now);
	/* precision: tenths of milliseconds */
	obj->triptime = now.tv_sec * 10000 + (now.tv_usec / 100);

	return len;
}


int icmp_close(struct libicmp *obj)
{
	if (!obj)
		return errno;

	if (obj->sd)
		close(obj->sd);

	free(obj->host);
	free(obj);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
