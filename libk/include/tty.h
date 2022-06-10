#ifndef __TTY_H__
#define __TTY_H__

#include <stddef.h>
#include <stdint.h>
#include <multiboot2.h>

void tty_init(multiboot_info_t multiboot_info, uint32_t);
void tty_puts(const char *, size_t, uint8_t);

#endif /* __TTY_H__ */
