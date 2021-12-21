PREFIX       := /usr/local
BOOTDIR      := /boot
INCLUDEDIR   := $(PREFIX)/include
ARCHDIR      := arch/$(HOSTARCH)

KERNEL_ARCH_CFLAGS  = -mgeneral-regs-only
KERNEL_ARCH_LDFLAGS =
KERNEL_ARCH_LIBS    =
KERNEL_ARCH_OBJS    =\
$(ARCHDIR)/gdtS.o\
$(ARCHDIR)/gdtc.o\
$(ARCHDIR)/flags.o\
$(ARCHDIR)/idt.o\
$(ARCHDIR)/boot.o\
$(ARCHDIR)/multiboot.o\

GUBOSINCLUDE := -Iinclude -I../libk/include -I$(ARCHDIR)/include -L=/usr/src/gubos/lib
CFLAGS       := $(GUBOSINCLUDE) $(KERNEL_ARCH_CFLAGS) -O2 -g -D__is_kernel -ffreestanding -Wall -Wextra
LIBS         := -nostdlib -lgcc -lk

KERNEL_OBJS = $(KERNEL_ARCH_OBJS) kernel/kernel.o

OBJS=\
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: gubos.kernel

gubos.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot2 gubos.kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

$(ARCHDIR)/gdtc.o:
	$(CC) -MD -c $(ARCHDIR)/gdt.c -o $(ARCHDIR)/gdtc.o -std=gnu11 $(CFLAGS)

$(ARCHDIR)/gdtS.o:
	$(CC) -MD -c $(ARCHDIR)/gdt.S -o $(ARCHDIR)/gdtS.o -std=gnu11 $(CFLAGS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

%.o: %.S
	$(CC) -MD -c $< -o $@ $(CFLAGS)

clean:
	rm -f gubos.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp -R include/. $(SYSROOT)$(INCLUDEDIR)/

install-kernel: gubos.kernel
	mkdir -p $(SYSROOT)$(BOOTDIR)
	cp gubos.kernel $(SYSROOT)$(BOOTDIR)
