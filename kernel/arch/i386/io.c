#ifndef __KIO_H__
#define __KIO_H__

#include <stdint.h>
#include "io.h"

void
io_wait() {
    // Write some data to an unused port to introduce a small delay
    for (int i = 0; i < 100000; ++i) {
        asm volatile("outb %al, $0x80");
    }
}

uint8_t
inb(uint16_t port) {
    uint8_t value = 0;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void
outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" :: "a"(value), "Nd"(port));
    io_wait();
}

#endif /* __KIO_H__ */
