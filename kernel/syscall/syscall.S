.globl syscall_interrupt_handler
.globl syscall_handler

syscall_interrupt_handler:
    push %ebp
    mov %esp, %ebp
    # create the registers_t argument
    push %ebp
    push %esp
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx
    push %eax
    # create the interrupt_state_t argument
    lea 0x4(%ebp), %eax
    push %eax
    # restore eax
    mov 0x4(%esp), %eax
    cld
    call syscall_handler
    leave
    iret
