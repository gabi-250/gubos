.PHONY: all clean

SRC  = $(wildcard *.s)
OBJS = $(SRC:.s=.bin)

all: $(OBJS)

%.o: %.s
	$(AS) $< -o $@

%.bin: %.o
	$(LD) $< -o $@

clean:
	rm -f $(OBJS)
