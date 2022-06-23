#include <stdint.h>

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <task.h>
#include <kmalloc.h>

extern vmm_context_t VMM_CONTEXT;

task_control_block_t *
task_create(uint32_t cr3, vmm_context_t *vmm_ctx, void (*task_fn)()) {
    static uint32_t last_pid = 0;

    uint32_t kernel_stack_bottom = (uint32_t)kmalloc(KERNEL_STACK_SIZE);
    // TODO: align stack top?
    uint32_t kernel_stack_top = kernel_stack_bottom + KERNEL_STACK_SIZE - 1;

    // Align the stack bottom
    kernel_stack_bottom += KERNEL_STACK_SIZE - (kernel_stack_bottom % KERNEL_STACK_SIZE);

    // If the stack is not mapped, you're going to have a bad time.
    for (size_t i = 0; i < KERNEL_STACK_SIZE / KERNEL_PAGE_SIZE_4KB; ++i) {
        uint32_t physical_addr = (uint32_t)pmm_alloc_page();
        paging_map_virtual_to_physical(kernel_stack_bottom + i * KERNEL_PAGE_SIZE_4KB, physical_addr,
                                       PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
    }
    task_control_block_t *task = (task_control_block_t *)kmalloc(sizeof(task_control_block_t));

    // EIP
    *(uint32_t *)kernel_stack_top = (uint32_t)task_fn;
    kernel_stack_top -= sizeof(uint32_t);
    // EBP
    *(uint32_t *)kernel_stack_top = 0;
    kernel_stack_top -= sizeof(uint32_t);
    // EBX
    *(uint32_t *)kernel_stack_top = 0;
    kernel_stack_top -= sizeof(uint32_t);
    // ESI
    *(uint32_t *)kernel_stack_top = 0;
    kernel_stack_top -= sizeof(uint32_t);
    // EDI
    *(uint32_t *)kernel_stack_top = 0;

    *task = (task_control_block_t) {
        .pid = last_pid++,
        .kernel_stack_top = kernel_stack_top,
        .virtual_addr_space = vmm_virtual_to_physical(cr3),
        .esp0 = kernel_stack_top,
        .vmm_context = vmm_ctx,
    };

    return task;
}
