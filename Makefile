# Makefile for libicmp and iping

CFLAGS    = -Wall -W -Werror -O4
ARFLAGS   = crus
LDLIBS    = $(LIBNAME)
RM        = rm -f

EXEC      = iping
LIBNAME   = libicmp.a

all: $(EXEC)

$(EXEC): libicmp.h $(LIBNAME)(libicmp.o)

$(LIBNAME)(libicmp.o): libicmp.h 

clean:
	-$(RM) *.o $(EXEC) $(LIBNAME)

distclean: clean
	-$(RM) *~ *.bak *.map .*.d DEADJOE semantic.cache *.gdb *.elf core core.*

