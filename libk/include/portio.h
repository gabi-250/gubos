#ifndef __PORTIO_H__
#define __PORTIO_H__

#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);
void io_wait();

#endif /* __PORTIO_H__ */
