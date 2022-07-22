.PHONY: all clean

SRC  = $(wildcard *.s)
OBJS = $(SRC:.s=.o)

all: $(OBJS)

%.o: %.s
	$(CC) -MD -c $< -o $@

%: %.o
	objcopy -O binary -j .text $< $@

clean:
	rm -f $(OBJS)
