.section .text

.globl _start
.globl exit

_start:
    call _init
    call main
    push %eax
    call exit
