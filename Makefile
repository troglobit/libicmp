CC	=gcc
CCFLAGS= -Wall -O4

all: testclient testserver

testclient: testclient.o libicmp.o
	$(CC) $(CCFLAGS) -o $@ testclient.o libicmp.o

testserver: testserver.o libicmp.o
	$(CC) $(CCFLAGS) -o $@ testserver.o libicmp.o

libicmp.o: libicmp.c libicmp.h
	$(CC) $(CCFLAGS) -c -o $@ libicmp.c

testserver.o: testserver.c libicmp.h
	$(CC) $(CCFLAGS) -c -o $@ testserver.c

testclient.o: testclient.c libicmp.h
	$(CC) $(CCFLAGS) -c -o $@ testclient.c

clean::
	rm -f *.o testserver testclient

