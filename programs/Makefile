.PHONY: all clean

SRC  = $(wildcard *.s)
OBJS = $(SRC:.s=.bin)

all: $(OBJS)

%.o: %.s
	$(CC) -MD -c $< -o $@

%.bin: %.o
	objcopy -O binary -j .text $< $@

clean:
	rm -f $(OBJS)
