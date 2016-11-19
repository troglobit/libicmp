/* Public Domain example of how to use libICMP */
#include "icmp.h"

int main(int argc, char *argv[])
{
	char *host = "localhost";
	struct libicmp *obj;

	if (argc >= 2)
		host = argv[1];

	if (!(obj = icmp_open(host, 0x1337, 0)))
		return 1;

	return icmp_ping(obj, 0, 0) == -1;
}
