.section .text

.globl _start
.globl exit

_start:
    call main
    push %eax
    call exit
    hlt
