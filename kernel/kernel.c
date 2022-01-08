#include <stdbool.h>

#include "tty.h"
#include "multiboot2.h"
#include "kernel_meminfo.h"
#include "printk.h"
#include "flags.h"
#include "ps2.h"
#include "mm/pmm.h"

__attribute__ ((constructor)) void
__init_kernel() {
    tty_init();
    ps2_init_devices();
}

__attribute__ ((destructor)) void
__uninit_kernel() {
    printk_debug("Fatal error: kernel returned.\n");
}

void
kernel_main(kernel_meminfo_t meminfo, multiboot_info_t multiboot_info) {
    if (multiboot_info.magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printk_debug("Invalid multiboot2 magic number: %d\n", multiboot_info.magic);
        return;
    }
    multiboot_print_info(multiboot_info.addr);
    printk_debug("Interrupts enabled: %s\n", interrupts_enabled() ? "yes" : "no");
    pmm_init(meminfo, multiboot_info);

    uint32_t module_addr;
    if ((module_addr = multiboot_get_first_module(multiboot_info.addr)) == 0) {
        printk_debug("Missing module\n");
        return;
    }
    printk_debug("module is at %#x\n", module_addr);
    printk_debug("kernel virtual start %#x\n", meminfo.virtual_start);
    printk_debug("kernel virtual end %#x\n", meminfo.virtual_end);
    printk_debug("kernel physical start %#x\n", meminfo.physical_start);
    printk_debug("kernel physical end %#x\n", meminfo.physical_end);
    // loop forever waiting for the next interrupt
    for(;;) {
        asm volatile("hlt");
    }
}
