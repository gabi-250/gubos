# Define the global constructors
#
# The kernel is compiled with -nostdlib which means we need to supply crti.o
# and crtn.o to the linker ourselves

.section .init
.global _init
_init:
    push %ebp
    movl %esp, %ebp

.section .fini
.global _fini
_fini:
    push %ebp
    movl %esp, %ebp
