#include <syscall/fork.h>
#include <task.h>

void
fork(registers_t *regs) {
    // TODO clone current task
    regs->eax = 0xbeef;
}
