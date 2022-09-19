.data
SYSCALL_NUM:
.long 0

.text
.globl _start
_start:
    mov $2, SYSCALL_NUM
    mov SYSCALL_NUM, %eax
    # fork
    int $80
    # exit syscall
    #mov $1, %eax
    mov $0xdeadbabe, %ebx
    mov $0xdead, %ecx
    mov $0xbabe, %ecx
    mov $0xde, %edx
    mov $0xad, %esi
    mov $0xbb, %edi
    int $80
