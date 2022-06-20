#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <mm/vmm.h>
#include <mm/vmm.h>

#define KERNEL_STACK_SIZE (1<<20)

typedef struct task_control_block {
    uint32_t pid;
    uint32_t kernel_stack_top;
    uint32_t virtual_addr_space;
    uint32_t esp0;
    vmm_context_t *vmm_context;
} task_control_block_t;

task_control_block_t *task_create(uint32_t, vmm_context_t *, void (*)());
void task_init(task_control_block_t *);

#endif /* __TASK_H__ */
