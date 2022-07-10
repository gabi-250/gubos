#include <stdint.h>

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <mm/addr_space.h>
#include <task.h>
#include <kmalloc.h>

task_control_block_t *
task_create(uint32_t cr3, vmm_context_t *vmm_ctx, void (*task_fn)(void)) {
    static uint32_t last_pid = 0;
    uint32_t kernel_stack_top = (uint32_t)alloc_kernel_stack(vmm_ctx);

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
