#include "tty.h"
#include "printk.h"

__attribute__ ((constructor)) void
__init_kernel() {
    init_tty();
    printk("kernel init complete\n");
}

void
kernel_main() {
    printk("%%lld: %lld hello\n", 123456789000);
    for (int i = 0; i < 5; ++i) {
        printk("Hello, kernel world %d!\n", i);
    }
}
