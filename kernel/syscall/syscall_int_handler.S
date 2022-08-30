.globl syscall_interrupt_handler
.globl syscall_handler

syscall_interrupt_handler:
    push %ebp
    mov %esp, %ebp
    # create the registers_t * argument
    push %ebp
    push %esp
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx
    push %eax
    lea (%esp), %eax
    push %eax
    # create the interrupt_state_t * argument
    lea 4(%ebp), %eax
    push %eax
    # restore eax
    mov 0x4(%esp), %eax
    cld
    call syscall_handler
    # pop the registers_t * argument
    pop %eax
    # pop the interrupt_state_t * argument
    pop %eax
    # pop the registers
    pop %eax
    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi
    pop %esp
    pop %ebp
    pop %ebp
    iret
