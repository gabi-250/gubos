CFLAGS?=-O2 -g
LDFLAGS?=

PREFIX := /usr/src/gubos
INCLUDEDIR := $(PREFIX)/include
LIBDIR := $(PREFIX)/lib

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -Iinclude
LIBK_CFLAGS:=$(CFLAGS) -D__is_libk

ARCHDIR=arch/$(HOSTARCH)

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)

OBJS=\
$(ARCH_OBJS) \
$(ARCHDIR)/tty.o \
$(ARCHDIR)/printk.o \
$(ARCHDIR)/string.o \

LIBK_OBJS=$(OBJS:.o=.libk.o)

BINARIES=libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S

all: $(BINARIES)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

%.o: %.S
	$(CC) -MD -c $< -o $@ $(CFLAGS)

%.libk.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS)

%.libk.o: %.S
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS)

clean:
	rm -f $(BINARIES) *.a
	rm -f $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(SYSROOT)$(LIBDIR)
	cp $(BINARIES) $(SYSROOT)$(LIBDIR)

-include $(LIBK_OBJS:.o=.d)
