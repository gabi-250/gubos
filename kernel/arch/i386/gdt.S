.globl load_gdt
.globl gdt

load_gdt:
    mov 4(%esp), %eax
    mov %eax, %ecx
    mov $gdt, %eax
    mov %eax, %ebx
    lgdt (%eax)
    mov $0x10, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss
    jmp $0x8, $.Lgdt_load_done
.Lgdt_load_done:
    ret
