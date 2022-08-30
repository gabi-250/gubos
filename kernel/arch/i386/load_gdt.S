.globl load_gdt
.globl load_tss

load_gdt:
    # The first argument is the GDT
    mov 4(%esp), %eax
    lgdt (%eax)
    # The third argument is the data segment selector: use it to initialize the
    # segment registers (except for CS, which needs to be intialized with the code
    # segment selector)
    mov 12(%esp), %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss
    # The second argument is the code segment selector: use it to create
    # a far jump to initialize the CS (code segment register)
    push 8(%esp)
    push $.Lgdt_load_done
    ljmp *(%esp)
.Lgdt_load_done:
    # Remove the code segement selector + offset we pushed before the long jump
    pop %eax
    pop %eax
    ret

load_tss:
    # The first argument is the segment selector that points to a TSS.
    mov 4(%esp), %eax
    ltr %ax
    ret
