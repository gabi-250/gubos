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

.PHONY: qemu monitor debug clean $(PROJECTS) $(PROGRAMS)

$(GUBOS): $(PROJECTS) $(PROGRAMS)
	./scripts/build.sh

qemu: $(GUBOS)
	qemu-system-i386 -cdrom $(GUBOS) $(QEMU_FLAGS)

monitor: $(GUBOS)
	qemu-system-i386 -cdrom $(GUBOS) -monitor stdio

debug: $(GUBOS)
	qemu-system-i386 -cdrom $(GUBOS) $(QEMU_FLAGS) -s -S &> qemu.log &
	gdb \
		--eval-command="layout split" \
		--eval-command="set history save on" \
		--eval-command="set arch i386" \
		--eval-command="target remote localhost:1234" \
		--eval-command="break _start" \
		--eval-command="break kernel_main" \
		./build/boot/gubos.kernel

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
