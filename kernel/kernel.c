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
#include <task.h>
#include <init.h>
#include <panic.h>

kernel_meminfo_t KERNEL_MEMINFO;
multiboot_info_t MULTIBOOT_INFO;
extern struct task_list CURRENT_TASK;

__attribute__ ((constructor)) void
__init_kernel() {
}

__attribute__ ((destructor)) void
__uninit_kernel() {
    printk_debug("Fatal error: kernel returned.\n");
}

static void
test_task() {
    /*for (int i = 0; i < 10; ++i) {*/
    for (;;) {
        printk_debug("task %u\n", CURRENT_TASK.task->pid);
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
    paging_context_t paging_ctx = init_paging();
    printk_debug("paging: OK\n");
    kmalloc_init();
    printk_debug("kmalloc: OK\n");

    vmm_context_t vmm_context = vmm_init();

    printk_debug("VMM: OK\n");

    struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_info.addr + 8);
    struct multiboot_tag_module *init_mod, *user_mod;

    if ((init_mod = multiboot_get_next_module(&tag)) == NULL) {
        printk_debug("init not found\n");
        return;
    }

    if ((user_mod = multiboot_get_next_module(&tag)) == NULL) {
        printk_debug("hello_userspace not found\n");
        return;
    }

    uint32_t init_mod_addr = init_mod->mod_start;
    printk_debug("init_mod is at %#x\n", init_mod_addr);
    uint32_t user_mod_addr = user_mod->mod_start;
    printk_debug("user_mod is at %#x\n", user_mod_addr);

    /*kmalloc_some_ints();*/

    init_sched(paging_ctx, vmm_context);
    printk_debug("scheduler init: OK\n");

    task_control_block_t *init_task = init_create_task0(paging_ctx, vmm_context, (void *)init_mod_addr);

    task_control_block_t *child = init_create_user_task(paging_ctx, vmm_context, (void *)user_mod_addr,
                                  init_task);

    for (size_t i = 0; i < 3; ++i) {
        task_control_block_t *task = task_create(paging_ctx,
                                     vmm_context, test_task, NULL, false);

        sched_add(task, TASK_PRIORITY_LOW);
    }

    sched_add(init_task, TASK_PRIORITY_LOW);
    sched_add(child, TASK_PRIORITY_LOW);

    for (;;) {
        printk_debug("task %u\n", CURRENT_TASK.task->pid);
        asm volatile("hlt");
    }

    PANIC("kernel task returned");
}
