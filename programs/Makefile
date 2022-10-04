.PHONY: all clean

SRC  = $(wildcard *.[sc])
OBJS = $(SRC:.c=.bin) $(SRC:.s=.bin)

all: $(OBJS)

%.bin: %.c
	$(CC) $< -o $@

%.o: %.s
	$(AS) $< -o $@

%.bin: %.o
	$(LD) $< -o $@

clean:
	rm -f *.bin
