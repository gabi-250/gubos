#ifndef __TTY_H__
#define __TTY_H__

#include <stddef.h>
#include <stdint.h>

void tty_init(void);
void tty_puts(const char *, size_t, uint8_t);

#endif /* __TTY_H__ */
