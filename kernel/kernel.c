#include <stdbool.h>

#include <tty.h>
#include <multiboot2.h>
#include <mm/meminfo.h>
#include <printk.h>
#include <flags.h>
#include <ps2.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <kmalloc.h>
#include <sched.h>

kernel_meminfo_t KERNEL_MEMINFO;
multiboot_info_t MULTIBOOT_INFO;

__attribute__ ((constructor)) void
__init_kernel() {
}

__attribute__ ((destructor)) void
__uninit_kernel() {
    printk_debug("Fatal error: kernel returned.\n");
}

void
kernel_main(kernel_meminfo_t meminfo, multiboot_info_t multiboot_info) {
    KERNEL_MEMINFO = meminfo;
    MULTIBOOT_INFO = multiboot_info;
    tty_init(meminfo.higher_half_base);
    ps2_init_devices();
    if (multiboot_info.magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printk_debug("Invalid multiboot2 magic number: %d\n", multiboot_info.magic);
        return;
    }
    multiboot_print_info(multiboot_info.addr);
    printk_debug("Interrupts enabled: %s\n", interrupts_enabled() ? "yes" : "no");
    printk_debug("kernel virtual start %#x\n", meminfo.virtual_start);
    printk_debug("kernel virtual end %#x\n", meminfo.virtual_end);
    printk_debug("kernel physical start %#x\n", meminfo.physical_start);
    printk_debug("kernel physical end %#x\n", meminfo.physical_end);

    printk_debug("Initializing memory manager\n");
    pmm_init(multiboot_info);
    printk_debug("PMM: OK\n");
    init_paging(meminfo);
    printk_debug("paging: OK\n");
    kmalloc_init();
    printk_debug("kmalloc: OK\n");
    vmm_init();
    printk_debug("VMM: OK\n");

    uint32_t module_addr;
    if ((module_addr = multiboot_get_first_module(multiboot_info.addr)) == 0) {
        printk_debug("Missing module\n");
        return;
    }
    printk_debug("module is at %#x\n", module_addr);

    for (int i = 0; i < 500; ++i) {
        int *x = (int *)kmalloc(sizeof(int));
        *x = 4;
        printk_debug("i=%d kmalloc'd x @ %#x. x is %d\n", i, (uint32_t)x, *x);
        long long *y = (long long *)kmalloc(sizeof(long long));
        *y = 4;
        printk_debug("kmalloc'd x @ %#x. y is %lld\n", y, *y);
        /*kfree(x);*/
        /*kfree(y);*/
    }

    init_sched();

    // loop forever waiting for the next interrupt
    for(;;) {
        asm volatile("hlt");
    }
}
