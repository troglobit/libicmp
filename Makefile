# Makefile for libicmp and iping

CC	= gcc
EXEC    = iping
CFLAGS  = -Wall -W -Werror -O4
LIBNAME = libicmp.a
LDLIBS  = $(LIBNAME)

all: $(EXEC)

$(EXEC): libicmp.h $(LIBNAME)

$(LIBNAME): libicmp.o
	ar crus $@ $^

#libicmp.o: libicmp.c libicmp.h

clean:
	-@rm -f *.o $(EXEC) $(LIBNAME)

distclean: clean
	-@rm -f *~ *.bak *.map .*.d DEADJOE semantic.cache *.gdb *.elf core core.*

