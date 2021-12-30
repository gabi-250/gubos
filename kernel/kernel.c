#include <stdbool.h>

#include "tty.h"
#include "multiboot2.h"
#include "printk.h"
#include "flags.h"
#include "ps2.h"

__attribute__ ((constructor)) void
__init_kernel() {
    tty_init();
    ps2_init_devices();
}

__attribute__ ((destructor)) void
__uninit_kernel() {
    printk("Fatal error: kernel returned.\n");
}

void
kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    if (multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printk("Invalid multiboot2 magic number: %d\n", multiboot_magic);
        return;
    }
    multiboot_print_info(multiboot_info);
    printk("Interrupts enabled: %s\n", interrupts_enabled() ? "yes" : "no");
    // loop forever waiting for the next interrupt
    for(;;) {
        asm volatile("hlt");
    }
}
