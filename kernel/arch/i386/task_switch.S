#define __ASSEMBLY__ 1
#include "flags.h"
#include "gdt.h"
#include "task.h"
#include "mm/addr_space.h"

.globl do_task_switch
.globl CURRENT_TASK
.globl tss
.globl halt_or_crash

# void do_task_switch(task_control_block_t *next);
do_task_switch:
    cli
    # eax = next
    mov 4(%esp), %eax
    push %ebp
    mov %esp, %ebp
    # Push the callee-saved registers
    push %ebx
    push %esi
    push %edi
    # Save the ESP of the previous task (CURRENT_TASK->kernel_stack_top = ESP)
    mov CURRENT_TASK, %ebx
    mov %esp, 4(%ebx)
    # CURRENT_TASK = next
    mov %eax, CURRENT_TASK
    # Load the next task
    # ESP = next->kernel_stack_top
    mov 4(%eax), %esp
    # ECX = next->virtual_addr_space (CR3)
    mov 8(%eax), %ecx
    # EDX = next->esp0
    mov 12(%eax), %edx
    # Update the ESP0 (the privilege level 0 stack pointer field) of the TSS
    # (located 32 bits into the structure).
    # NOTE: TSS->ESP0 is only used when switching from user-mode to ring 0 (it
    # represents the kernel stack of the task). The switching happens, for example,
    # whenever a user-space task is interrupted by an interrupt (ESP0 is the stack
    # pointer used by the interrupt handler).
    # TSS->ESP0 = next->TSS0
    lea tss, %esi
    mov %edx, 4(%esi)
    mov %cr3, %ebx
    # Avoid reloading CR3 unless the new value is different from the previous
    # (the TLB is flushed if you write to CR3).
    cmp %ebx, %ecx
    je .Ldone
    # Load the CR3 of the next task
    mov %ecx, %cr3
.Ldone:
    # Restore the callee-saved registers
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    sti
    ret

# void halt_or_crash();
halt_or_crash:
    hlt
    # We should never reach this point, because after a task returns it is
    # removed from the scheduler's queue.
    ud2
