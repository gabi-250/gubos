PREFIX                := /usr/src/memo
INCLUDEDIR            := $(PREFIX)/include
LIBDIR                := $(PREFIX)/lib
LDFLAGS               :=
MEMOINCLUDE          :=\
-Iinclude\
-I../kernel/include\

CFLAGS               := -O2 -g -ffreestanding -std=gnu2x -Wall -Wextra -Werror -pedantic $(MEMOINCLUDE)
LIBDRIVERS_CFLAGS    := $(CFLAGS)
OBJS                 := pic.o ps2.o
LIBDRIVERS_OBJS      := $(OBJS:.o=.libdrivers.o)
BINARIES             :=libdrivers.a

.PHONY: all clean install install-libs
.SUFFIXES: .o .libdrivers.o .c .S

all: $(BINARIES)

libdrivers.a: $(LIBDRIVERS_OBJS)
	$(AR) rcs $@ $(LIBDRIVERS_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MD -c $< -o $@

%.o: %.S
	$(CC) $(LDFLAGS) -MD -c $< -o $@ $(CFLAGS)

%.libdrivers.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MD -c $< -o $@

%.libdrivers.o: %.S
	$(CC) $(CFLAGS) $(LDFLAGS) -MD -c $< -o $@

clean:
	rm -f $(BINARIES) *.a
	rm -f $(LIBDRIVERS_OBJS) *.o */*.o */*/*.o
	rm -f $(LIBDRIVERS_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-libs

install-libs: $(BINARIES)
	mkdir -p $(SYSROOT)$(LIBDIR)
	cp $(BINARIES) $(SYSROOT)$(LIBDIR)

-include $(LIBDRIVERS_OBJS:.o=.d)
