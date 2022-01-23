.PHONY: all clean

SRC  = $(wildcard *.s)
OBJS = $(SRC:.s=.o)

all: $(OBJS)

%.o: %.s
	$(CC) -MD -c $< -o $@

clean:
	rm -f $(OBJS)
