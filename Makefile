# Makefile for libicmp and iping
CC	= gcc
CFLAGS  = -Wall -O4
LDLIBS  = libicmp.o

all: $(LDLIBS) iping

test: $(LDLIBS) testclient testserver

iping: iping.o

testclient: testclient.o

testserver: testserver.o

$(LDLIBS): libicmp.c libicmp.h

testserver.o: testserver.c libicmp.h

testclient.o: testclient.c libicmp.h

clean:
	-@rm -f *.o testserver testclient

