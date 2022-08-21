#include <syscall/exit.h>
#include <sched.h>
#include <printk.h>
#include <panic.h>
#include <registers.h>

extern struct task_list CURRENT_TASK;

void
exit(registers_t *regs) {
    uint32_t pid = CURRENT_TASK.task->pid;

    sched_remove(pid);

    if (!CURRENT_TASK.task->parent) {
        PANIC("init task exited");
    } else {
        // TODO: return from the interrupt into the parent task
        uint32_t *ret_addr = (uint32_t *)(regs->ebp + sizeof(uint32_t));
        *ret_addr = (uint32_t)halt; // XXX
    }

    printk_debug("task %u exited with status %u\n", pid, regs->eax);
}
