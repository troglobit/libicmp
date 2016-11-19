# Makefile for libICMP and iping

CC        = @gcc
CFLAGS    = -Wall -W -Wextra
AR        = @ar
ARFLAGS   = crUs
LDFLAGS   = -L.
LDLIBS    = -licmp
RM        = @rm -f

EXEC      = iping
OBJS      = iping.o libicmp.o
DEPS      = $(addprefix .,$(OBJS:.o=.d))

LIBNAME   = libicmp.a
DEPLIB    = $(LIBNAME)(libicmp.o)

# Smart autodependecy generation via GCC -M.
.%.d: %.c
	@$(SHELL) -ec "$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< 2>/dev/null \
                | sed 's,.*: ,$*.o $@ : ,g' > $@; \
                [ -s $@ ] || rm -f $@"

all: $(EXEC)

$(EXEC): $(LIBNAME)

$(LIBNAME): $(DEPLIB)

clean:
	-$(RM) *.o $(EXEC) $(LIBNAME)

distclean: clean
	-$(RM) *~ *.bak *.map .*.d DEADJOE semantic.cache *.gdb *.elf core core.*

-include $(DEPS)
