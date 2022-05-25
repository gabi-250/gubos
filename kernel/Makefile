PREFIX                := /usr/local
BOOTDIR               := /boot
INCLUDEDIR            := $(PREFIX)/include
ARCHDIR               := arch/$(HOSTARCH)
KERNEL_ARCH_CFLAGS    := -mgeneral-regs-only
KERNEL_ARCH_LDFLAGS   :=
KERNEL_ARCH_LIBS      :=
LIBS                  := -ldrivers -lk -nostdlib -lgcc
CFLAGS                = $(GUBOSINCLUDE) $(KERNEL_ARCH_CFLAGS) -O0 -g -D__is_kernel -ffreestanding -Wall -Wextra -Werror
LDFLAGS               =

GUBOSINCLUDE :=\
-Iinclude\
-I../libk/include\
-I../drivers/include\
-I$(ARCHDIR)/include\
-L=/usr/src/gubos/lib\

KERNEL_ARCH_OBJS :=\
$(ARCHDIR)/gdtS.o\
$(ARCHDIR)/gdtc.o\
$(ARCHDIR)/flags.o\
$(ARCHDIR)/interrupts/idt.o\
$(ARCHDIR)/interrupts/page_fault.o\
$(ARCHDIR)/interrupts/handlers.o\
$(ARCHDIR)/boot.o\
$(ARCHDIR)/multiboot2_header.o\
$(ARCHDIR)/multiboot2.o\
$(ARCHDIR)/mm/pmm.o\
$(ARCHDIR)/mm/vmm.o\
$(ARCHDIR)/init_paging.o\

KERNEL_OBJS = $(KERNEL_ARCH_OBJS) kernel.o kmalloc.o

OBJS =\
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
