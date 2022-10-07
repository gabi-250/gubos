.PHONY: all clean

SRC  = $(wildcard *.[sc])
OBJS = $(SRC:.c=.bin) $(SRC:.s=.bin)

CFLAGS = -O0 -g -Wall -Wextra -Werror

all: $(OBJS)

%.bin: %.c
	$(CC) $< -o $@ $(CFLAGS)

%.o: %.s
	$(AS) $< -o $@

%.bin: %.o
	$(LD) $< -o $@

clean:
	rm -f *.bin
