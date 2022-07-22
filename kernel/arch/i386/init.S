#define __ASSEMBLY__ 1
#include "flags.h"
#include "gdt.h"
#include "task.h"
#include "mm/addr_space.h"

.globl init_goto_user_mode

# void init_goto_user_mode(uint32_t user_mode_addr);
init_goto_user_mode:
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
