#include <stdbool.h>

#include "tty.h"
#include "printk.h"
#include "flags.h"
#include "ps2.h"

__attribute__ ((constructor)) void
__init_kernel() {
    init_tty();
    init_ps2_devices();
    printk("Kernel init complete\n");
}

__attribute__ ((destructor)) void
__uninit_kernel() {
    printk("Kernel returning\n");
}

void
kernel_main() {
    printk("Interrupts enabled: %s\n", interrupts_enabled() ? "yes" : "no");
    // loop forever waiting for the next interrupt
    for(;;) {
        asm volatile("hlt");
    }
}
