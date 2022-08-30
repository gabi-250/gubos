PREFIX                := /usr/local
BOOTDIR               := /boot
INCLUDEDIR            := $(PREFIX)/include
ARCHDIR               := arch/$(HOSTARCH)
KERNEL_ARCH_CFLAGS    := -mgeneral-regs-only
KERNEL_ARCH_LDFLAGS   :=
KERNEL_ARCH_LIBS      :=
LIBS                  := -ldrivers -nostdlib -lgcc
CFLAGS                = $(GUBOSINCLUDE) $(KERNEL_ARCH_CFLAGS) -O0 -g -D__is_kernel -ffreestanding -Wall -Wextra -Werror
LDFLAGS               =

GUBOSINCLUDE :=\
-Iinclude\
-I../drivers/include\
-I$(ARCHDIR)/include\
-L=/usr/src/gubos/lib\

KERNEL_ARCH_SRCS := $(wildcard $(ARCHDIR)/*.c $(ARCHDIR)/*.S $(ARCHDIR)/**/*.c $(ARCHDIR)/**/*.S)
KERNEL_ARCH_OBJS := $(addsuffix .o, $(basename $(KERNEL_ARCH_SRCS)))

KERNEL_SRCS := $(wildcard *.c string/*.c syscall/*.c syscall/*.S)
KERNEL_OBJS := $(addsuffix .o, $(basename $(KERNEL_SRCS)))

OBJS =\
$(KERNEL_ARCH_OBJS) \
$(KERNEL_OBJS) \

LINK_LIST=\
$(LDFLAGS) \
$(KERNEL_ARCH_OBJS) \
$(KERNEL_OBJS) \
$(LIBS) \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: gubos.kernel

gubos.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot2 gubos.kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

%.o: %.S
	$(CC) -MD -c $< -o $@ $(CFLAGS)

string/%.o: string/%.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

$(ARCHDIR)/%.o: $(ARCHDIR)/%.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

$(ARCHDIR)/%.o: $(ARCHDIR)/%.S
	$(CC) -MD -c $< -o $@ $(CFLAGS)

$(ARCHDIR)/mm/%.o: $(ARCHDIR)/mm/%.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

$(ARCHDIR)/mm/%.o: $(ARCHDIR)/mm/%.S
	$(CC) -MD -c $< -o $@ $(CFLAGS)

$(ARCHDIR)/interrupts/%.o: $(ARCHDIR)/interrupts/%.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

$(ARCHDIR)/interrupts/%.o: $(ARCHDIR)/interrupts/%.S
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
