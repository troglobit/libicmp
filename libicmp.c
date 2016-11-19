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

static u_short
in_cksum(u_short * buf, int nwords)
{
    u_long sum;

    for (sum = 0; nwords > 0; nwords--)
	sum += *buf++;

    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}


/*
 * If your opening up a socket for listening, set both paramaters to 0
 */
libicmp_t *
icmp_open(char *host, unsigned short id, unsigned ttl)
{
    libicmp_t       *isock;
    socklen_t        ttl_len;
    struct addrinfo *addr;

    if (icmp_resolve(host, &addr))
	return NULL;

    isock = malloc(sizeof(libicmp_t));
    if (!isock) {
	freeaddrinfo(addr);
	return NULL;
    }

    bzero(isock, sizeof(libicmp_t));

    isock->sd   = socket(AF_INET, SOCK_RAW, 1);
    isock->id   = id;
    isock->host = strdup(host);

    if (isock->sd < 0) {
	free(isock->host);
	free(isock);
	return NULL;
    }

    if (ttl) {
	ttl_len = sizeof(isock->ttl);
	setsockopt(isock->sd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	setsockopt(isock->sd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	getsockopt(isock->sd, IPPROTO_IP, IP_TTL, &isock->ttl, &ttl_len);
    }

    return isock;
}

int
icmp_resolve(char *host, struct addrinfo **addr)
{
    int code;
    struct addrinfo hints;

    res_init(); /* Reinitialize resolver every time to prevent cache misses due to
		 * NAME-->IP DNS changes, or outages in round robin DNS setups. */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    code = getaddrinfo(host, NULL, &hints, addr);
    if (code) {
	errno = EINVAL;
	return code;
    }

    return 0;
}

char *
icmp_ntoa(struct addrinfo *addr, char *buf, size_t len)
{
    /* NI_NUMERICHOST avoids DNS lookup. */
    if (getnameinfo(addr->ai_addr, addr->ai_addrlen, buf, len, NULL, 0, NI_NUMERICHOST))
	return NULL;

    return buf;
}

int
icmp_send(libicmp_t *isock, u_int8_t type, char *payload, size_t len)
{
    int              i;
    char             buffer[BUFSIZE];
    struct icmphdr  *icmp = (struct icmphdr *)buffer;
    struct timeval   now;
    struct addrinfo *addr;

    if (!isock) {
	errno = EINVAL;
	return 0;
    }

    isock->gai_code = icmp_resolve(isock->host, &addr);
    if (isock->gai_code)
	return 0;

    bzero(buffer, BUFSIZE);
    gettimeofday(&now, NULL);

    /* ICMP Payload is our time now + any user defined payload */
    memcpy((buffer + sizeof(struct icmphdr)), &now, sizeof(struct timeval));
    memcpy((buffer + sizeof(struct icmphdr) + sizeof(struct timeval)), payload, len);

    icmp->type = type;
    icmp->un.echo.id = htons(isock->id);
    icmp->un.echo.sequence = htons(isock->seqno);
    icmp->checksum = in_cksum((u_short *)icmp, sizeof(struct icmphdr) + sizeof(struct timeval) + len);

    i = sendto(isock->sd, buffer, sizeof(struct icmphdr) + sizeof(struct timeval) + len, 0,
	       addr->ai_addr, sizeof(struct sockaddr));
    freeaddrinfo(addr);
    if (i < 0) {
	perror("sendto");
	return 0;
    }

    return i;
}


size_t
icmp_recv(libicmp_t *isock, char *buf, u_int8_t type, int timeout)
{
    int             i, checksum;
    char           *ptr;
    char            buffer[BUFSIZE];
    struct iphdr   *ip   = (struct iphdr *)buffer;
    struct icmphdr *icmp = (struct icmphdr *)(buffer + sizeof(struct iphdr));

    if (!isock) {
	errno = EINVAL;
	return 0;
    }

    ptr = buffer + sizeof(struct iphdr) + sizeof(struct icmphdr);
    while (1) {
	int ret = 0;
	struct pollfd pfd = { isock->sd, POLLIN | POLLPRI, 0 };

	ret = poll(&pfd, 1, timeout);
	if (ret <= 0) {
	    if (ret == 0)
		errno = ETIMEDOUT;

	    return 0;
	}

	if (pfd.revents & (POLLIN | POLLPRI)) {
	    size_t datalen;

	    i = read(isock->sd, buffer, BUFSIZE);
	    if (i < 0)
		return 0;

	    if (ip->protocol != 1 || icmp->type != type ||
		icmp->un.echo.id != htons(isock->id) ||
		icmp->un.echo.sequence != htons(isock->seqno))
		continue;

	    checksum       = icmp->checksum;
	    icmp->checksum = 0;
	    datalen        = i - sizeof(struct iphdr);
	    if (checksum != in_cksum((u_short *)icmp, datalen)) {
		errno = EIO;
		return 0;
	    }

	    memcpy(&isock->tv, ptr, sizeof(struct timeval));
	    memcpy(buf, ptr + sizeof(struct timeval), datalen - sizeof(struct timeval));

	    return datalen;
	}
    }
}


int
icmp_ping(libicmp_t *isock, char *payload, size_t len)
{
    struct timeval now;
    char buf[MAX_DGRAM_LEN];

    isock->seqno++;
    if (!icmp_send(isock, ICMP_ECHO, payload, len))
	return 0;

    len = icmp_recv(isock, buf, ICMP_ECHOREPLY, 5000);
    if (!len) {
	return 0;
    }

    gettimeofday(&now, NULL);
    timersub(&now, &isock->tv, &now);
    /* precision: tenths of milliseconds */
    isock->triptime = now.tv_sec * 10000 + (now.tv_usec / 100);

    return len;
}


int
icmp_close(libicmp_t * isock)
{
    if (!isock)
	return errno;

    if (isock->sd)
	close(isock->sd);

    free(isock->host);
    free(isock);

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
