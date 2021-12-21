#include <stdint.h>
#include <stdbool.h>
#include "flags.h"

uint32_t
cpu_flags() {
    uint32_t flags = 0;
    asm volatile("pushf\n\t"
                 "pop %0"
                  : "=g"(flags));
    return flags;
}


bool
interrupts_enabled() {
    return (cpu_flags() & FLAGS_INT_ENABLE_FLAG) != 0;
}
