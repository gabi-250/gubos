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
#include <mm/addr_space.h>
#include <kmalloc.h>
#include <sched.h>
#include <init.h>
#include <panic.h>

kernel_meminfo_t KERNEL_MEMINFO;
multiboot_info_t MULTIBOOT_INFO;

__attribute__ ((constructor)) void
__init_kernel() {
}

__attribute__ ((destructor)) void
__uninit_kernel() {
    printk_debug("Fatal error: kernel returned.\n");
}

static void
test_task1() {
    for (int i = 0; i < 10; ++i) {
        printk_debug("task #1 here\n");
        asm volatile("hlt");
    }
}

static void
test_task2() {
    for (int i = 0; i < 5; ++i) {
        printk_debug("task #2 here\n");
        asm volatile("hlt");
    }
}

static void
test_task3() {
    for (;;) {
        printk_debug("task #3 here\n");
        asm volatile("hlt");
    }
}

__attribute__((unused)) static void
kmalloc_some_ints() {
    for (int i = 0; i < 1000; ++i) {
        int *x = (int *)kmalloc(sizeof(int));
        *x = 4;
        printk_debug("i=%d kmalloc'd x @ %#x. x is %d\n", i, (uint32_t)x, *x);
        long long *y = (long long *)kmalloc(sizeof(long long));
        *y = 4;
        printk_debug("kmalloc'd x @ %#x. y is %lld\n", y, *y);
        kfree(x);
        kfree(y);
    }
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
    printk_debug("kernel stack top %#x\n", meminfo.stack_top);

    printk_debug("Initializing memory manager\n");
    pmm_init(multiboot_info);
    printk_debug("PMM: OK\n");
    paging_context_t paging_ctx = init_paging(meminfo);
    printk_debug("paging: OK\n");
    kmalloc_init();
    printk_debug("kmalloc: OK\n");

    vmm_context_t vmm_context = vmm_init();

    printk_debug("VMM: OK\n");

    struct multiboot_tag_module *module;
    if ((module = multiboot_get_first_module(multiboot_info.addr)) == 0) {
        printk_debug("Missing module\n");
        return;
    }

    uint32_t module_addr = module->mod_start;
    printk_debug("module is at %#x\n", module_addr);

    /*kmalloc_some_ints();*/

    init_sched(paging_ctx, vmm_context);

    task_control_block_t *task1 = task_create(paging_ctx, vmm_context, test_task1, NULL, false);
    task_control_block_t *task2 = task_create(paging_ctx, vmm_context, test_task2, NULL, false);
    task_control_block_t *task3 = task_create(paging_ctx, vmm_context, test_task3, NULL, false);
    sched_add(task1, TASK_PRIORITY_LOW);
    sched_add(task2, TASK_PRIORITY_LOW);
    sched_add(task3, TASK_PRIORITY_LOW);

    task_control_block_t *init_task = init_create_task0(paging_ctx, vmm_context, (void *)module_addr);
    sched_add(init_task, TASK_PRIORITY_LOW);

    for (;;) {
        asm volatile("hlt");
    }
    PANIC("kernel task returned");
}
