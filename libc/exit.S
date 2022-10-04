.section .text

.globl exit
.globl abort

exit:
    mov 4(%esp), %ebx
    # exit syscall
    mov $1, %eax
    int $80

abort:
    jmp abort
