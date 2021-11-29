CFLAGS     ?= -O2 -g -ffreestanding -Wall -Wextra -Iinclude

PREFIX     := /usr/local
INCLUDEDIR := $(PREFIX)/include
LIBDIR     := $(PREFIX)/lib

OBJS := stdio/printf.o stdio/putchar.o stdio/puts.o\
	stdlib/abort.o string/memcmp.o string/memcpy.o\
	string/memmove.o string/memset.o string/strlen.o

BINARIES = libc.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libc.o .c .S

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

%.o: %.S
	$(CC) -MD -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp -R include/. $(SYSROOT)$(INCLUDEDIR)/

install-libs: $(BINARIES)
	mkdir -p $(SYSROOT)$(LIBDIR)
	cp $(BINARIES) $(SYSROOT)$(LIBDIR)
