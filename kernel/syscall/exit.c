#include <syscall/exit.h>
#include <sched.h>
#include <printk.h>
#include <panic.h>
#include <registers.h>

extern struct task_list CURRENT_TASK;

void
exit(registers_t *regs) {
    uint32_t pid = CURRENT_TASK.task->pid;

    if (!CURRENT_TASK.task->parent) {
        PANIC("init task exited");
    }

    sched_remove(pid);

    // TODO: return from the interrupt into the parent task
    printk_debug("task %u exited with status %#x\n", pid, regs->ebx);
}
