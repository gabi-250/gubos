#include <task.h>
#include <sched.h>
#include <kmalloc.h>
#include <printk.h>
#include <mm/vmm.h>
#include <mm/meminfo.h>

#include <stddef.h>

extern kernel_meminfo_t KERNEL_MEMINFO;
extern page_table_t ACTIVE_PAGE_DIRECTORY;
extern vmm_context_t VMM_CONTEXT;

task_list_t SCHED_TASKS;

task_control_block_t *current_task;

static void
test_task() {
    // Switch back to the first task
    for (;;) {
        printk_debug("task #2 here\n");
        sched_switch_task(SCHED_TASKS.next);
    }
}

static void
first_task() {
    task_control_block_t *new_task = task_create((uint32_t)&ACTIVE_PAGE_DIRECTORY, &VMM_CONTEXT,
                                     test_task);
    for (;;) {
        printk_debug("first task\n");
        sched_switch_task(new_task);
    }
}

void
init_sched() {
    current_task = task_create((uint32_t)&ACTIVE_PAGE_DIRECTORY, &VMM_CONTEXT, NULL);
    task_control_block_t *init_task = task_create((uint32_t)&ACTIVE_PAGE_DIRECTORY, &VMM_CONTEXT,
                                      first_task);
    // Create the first kernel task
    SCHED_TASKS = (task_list_t) {
        .task = current_task,
        .next = init_task,
    };
    // Start executing it
    sched_switch_task(SCHED_TASKS.task);
    sched_switch_task(SCHED_TASKS.next);
}
