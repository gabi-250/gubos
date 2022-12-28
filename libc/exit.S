.section .text

#include "syscall.h"

.globl exit
.globl abort

exit:
    mov 4(%esp), %ebx
    mov $SYS_EXIT, %eax
    int $80

abort:
    jmp abort
