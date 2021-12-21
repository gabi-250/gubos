#include <stdbool.h>

#include "tty.h"
#include "printk.h"
#include "flags.h"

__attribute__ ((constructor)) void
__init_kernel() {
    init_tty();
    printk("Kernel init complete\n");
}

void
kernel_main() {
    printk("Interrupts enabled: %s\n", interrupts_enabled() ? "yes" : "no");
    asm volatile ("int $49");
    printk("%%lld demo: %lld hello\n", 123456789000);
    for (int i = 0; i < 5; ++i) {
        printk("Hello, kernel world %d!\n", i);
    }
}
