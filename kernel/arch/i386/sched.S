#define __ASSEMBLY__ 1
#include "flags.h"
#include "gdt.h"
#include "task.h"
#include "mm/addr_space.h"

.globl do_task_switch
.globl current_task
.globl tss
.globl goto_user_mode

# void do_task_switch(task_control_block_t *next);
do_task_switch:
    # eax = next
    mov 4(%esp), %eax
    push %ebp
    mov %esp, %ebp
    # Push the callee-saved registers
    push %ebx
    push %esi
    push %edi
    # Save the ESP of the previous task (current_task->kernel_stack_top = ESP)
    mov current_task, %ebx
    mov %esp, 4(%ebx)
    # current_task = next
    mov %eax, current_task
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
    mov %ebx, %cr3
.Ldone:
    # Restore the callee-saved registers
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret

# void goto_user_mode(uint32_t user_mode_addr);
goto_user_mode:
    cli
    mov $(GDT_USER_DATA_SEGMENT | GDT_USER_RPL), %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    # get the return address
    mov 4(%esp), %esi
    # When returning from an interrupt, the top of the stack is supposed to
    # look like this:
    #  +---------------------+
    #  |        SS:ESP       |
    #  +---------------------+
    #  |        EFLAGS       |
    #  +---------------------+
    #  |        CS:EIP       |
    #  +---------------------+
    #  | err_code (optional) |
    #  +---------------------+
    # The userspace stack
    push $(GDT_USER_DATA_SEGMENT | GDT_USER_RPL)
    push $USER_STACK_TOP
    pushf
    # Set the IF flag of EFLAGS to enable interrupts for the userspace task
    or $FLAGS_INT_ENABLE_FLAG, (%esp)
    push $(GDT_USER_CODE_SEGMENT | GDT_USER_RPL)
    push %esi
    iret
