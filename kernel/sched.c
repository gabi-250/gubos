#include <task.h>
#include <init.h>
#include <sched.h>
#include <gdt.h>
#include <flags.h>
#include <kmalloc.h>
#include <printk.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <mm/meminfo.h>
#include <panic.h>

#include <stddef.h>

extern kernel_meminfo_t KERNEL_MEMINFO;
extern page_table_t ACTIVE_PAGE_DIRECTORY;

extern void do_task_switch(task_control_block_t *next);
extern void halt_or_crash();

int SCHED_INIT = 0;

struct task_list CURRENT_TASK;
static struct task_list *tasks_head, *tasks_tail, *tasks_sched_head;

static void
sched_switch_task(task_control_block_t *next) {
    do_task_switch(next);
}

void
init_sched(paging_context_t paging_ctx, vmm_context_t vmm_context) {
    task_control_block_t *task = task_create(paging_ctx, vmm_context, NULL, NULL, false);
    tasks_sched_head = tasks_head = tasks_tail = kmalloc(sizeof(struct task_list));

    // Create the first kernel task
    *tasks_head = (struct task_list) {
        .next = tasks_head,
        .task = task,
        .priority = TASK_PRIORITY_LOW,
    };

    CURRENT_TASK = *tasks_head;
    // Start executing it
    sched_switch_task(CURRENT_TASK.task);
    // TODO: locking
    SCHED_INIT = 1;
}

void
sched_add(task_control_block_t *task, task_priority_t priority) {
    struct task_list *new_tasks = kmalloc(sizeof(struct task_list));
    *new_tasks = (struct task_list) {
        .next = tasks_tail->next,
        .task = task,
        .priority = priority,
    };
    tasks_tail->next = new_tasks;
    tasks_tail = new_tasks;
}

void
sched_remove(uint32_t pid) {
    if (!pid || pid == INIT_PID) {
        PANIC("cannot remove task (PID=%u)", pid);
    }

    struct task_list *task = tasks_tail;
    if (task->task->pid == pid) {
        tasks_tail = tasks_tail->next;
        tasks_head->next = tasks_tail;
        kfree(task);
        return;
    }

    struct task_list *prev;
    do {
        prev = task;
        task = tasks_tail->next;
        if (task->task->pid == pid) {
            prev->next = task->next;
            if (task == tasks_head) {
                tasks_head = task->next;
            }
            kfree(task);
            return;
        }
    } while (task != tasks_tail);

    PANIC("task %u not found", pid);
}

void
sched_context_switch() {
    task_control_block_t *task = tasks_sched_head->task;
    tasks_sched_head = tasks_sched_head->next;

    sched_switch_task(task);
}

__attribute__((noreturn)) void
sched_halt_or_crash() {
    halt_or_crash();
    __builtin_unreachable();
}
