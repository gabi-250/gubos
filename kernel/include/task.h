#ifndef __TASK_H__
#define __TASK_H__

#ifndef __ASSEMBLY__
#include <stdint.h>
#include <mm/vmm.h>
#include <mm/paging.h>

typedef struct task_control_block {
    uint32_t pid;
    uint32_t kernel_stack_top;
    uint32_t virtual_addr_space;
    uint32_t esp0;
    vmm_context_t vmm_context;
    paging_context_t paging_ctx;
    struct task_control_block *parent;
} task_control_block_t;

typedef enum sched_priority {
    TASK_PRIORITY_LOW,
} task_priority_t;

struct task_list {
    task_control_block_t *task;
    task_priority_t priority;
    struct task_list *next;
};

task_control_block_t *task_create(paging_context_t, vmm_context_t, void (*)(void), void *, bool);
void task_init(task_control_block_t *);
#endif

#endif /* __TASK_H__ */
