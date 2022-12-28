SRCS := $(wildcard *.c *.S **/*.c **/*.S)
OBJS := $(addsuffix .o, $(basename $(SRCS)))
LIBC_A := libc.a

CFLAGS = -O0 -g -std=gnu2x -Wall -Wextra -Werror -pedantic

.PHONY: all clean install install-headers

all: $(LIBC_A)

%.o: %.c
	$(CC) -MD -c $< -o $@ $(CFLAGS) -Iinclude

$(LIBC_A): install-headers $(OBJS)
	$(AR) rcs $(LIBC_A) $(OBJS)

install: $(LIBC_A) install-headers
	mkdir -p $(SYSROOT)$(LIBDIR)
	cp $(LIBC_A) $(SYSROOT)$(LIBDIR)
	cp crt0.o $(SYSROOT)$(LIBDIR)

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp -R include/. $(SYSROOT)$(INCLUDEDIR)/

clean:
	rm -rf $(OBJS) $(LIBC_A)
