#include <registers.h>
#include <interrupts/handlers.h>
#include <syscall/syscall.h>
#include <syscall/exit.h>
#include <printk.h>
#include <panic.h>

void
syscall_handler(interrupt_state_t *state, registers_t regs) {
    uint32_t syscall_num = regs.eax;

    printk_debug("handling syscall (eflags=%#x, cs=%d, eip=%d, syscall=%d)\n",
                 state->eflags,
                 state->cs,
                 state->eip,
                 syscall_num);

    printk_debug("registers eax=%#x ebx=%#x ecx=%#x edx=%#x esi=%#x edi=%#x\n",
                 regs.eax, regs.ebx, regs.ecx, regs.edx, regs.esi, regs.edi);

    switch (syscall_num) {
        case SYS_EXIT:
            exit(&regs);
            break;
        default:
            PANIC("unknown syscall %d", syscall_num);
    }
}
