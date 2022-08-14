GUBOS                  := gubos.bin
PROJECTS               := libk drivers kernel
PROGRAMS               := programs
SYSTEM_HEADER_PROJECTS := libk kernel
HOST                   := i686-elf
HOSTARCH               := i386
SYSROOT                := $(abspath sysroot)
AR                     := $(HOST)-ar
AS                     := $(HOST)-as
CC                     := $(HOST)-gcc --sysroot=$(SYSROOT)
GDB                    := $(HOST)-gdb --sysroot=$(SYSROOT)

# Export all variables to make them available to sub-make
export

QEMU_FLAGS             := -d int,cpu_reset --no-reboot --no-shutdown

.PHONY: qemu monitor debug clean fmt $(PROJECTS) $(PROGRAMS)

$(GUBOS): $(PROJECTS) $(PROGRAMS)
	./scripts/build.sh

qemu: $(GUBOS)
	qemu-system-i386 -cdrom $(GUBOS) $(QEMU_FLAGS)

monitor: $(GUBOS)
	qemu-system-i386 -cdrom $(GUBOS) -monitor stdio $(QEMU_FLAGS)

debug: $(GUBOS)
	qemu-system-i386 -cdrom $(GUBOS) $(QEMU_FLAGS) -s -S &> qemu.log &
	gdb ./build/boot/gubos.kernel -x ./scripts/gdb.txt
fmt:
	astyle \
		--break-return-type \
		--align-pointer=name \
		--style=attach \
		--indent-switches \
		--max-code-length=100 \
		--recursive './*.c,*.h' \

$(PROJECTS):
ifeq ($(MAKECMDGOALS), clean)
	@echo "cleaning $@"
	make -C $@ clean
else
	make -C $@ install
endif

$(PROGRAMS):
ifeq ($(MAKECMDGOALS), clean)
	@echo "cleaning $@"
	make -C $@ clean
endif

clean: $(PROJECTS) $(PROGRAMS)
	rm -rf sysroot build $(GUBOS) qemu.log
