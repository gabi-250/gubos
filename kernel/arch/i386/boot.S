.section .bss
.align 16

# Reserve a stack for the kernel
stack_bottom:
.skip 1 << 14
stack_top:

.section .data
# The page tables
.include "./arch/i386/include/page_directory.inc"
# A temporary GDT to replace the one set up by GRUB (only needed until paging is
# enabled, because once we have paging, the GDT setup can more easily be handled in
# C land).
.include "./arch/i386/include/gdt.inc"

.text
# The kernel entry point
.global _start
_start:
    # Disable interrupts for now
    cli
    # Set up a temporary GDT
    # Remember: at this point paging isn't enabled yet, so we need to subtract
    # 0xC0000000 from every memory access.
    lgdt tmp_gdt_descriptor - 0xC0000000
    jmp $TMP_GDT_CODE_SEGMENT, $.Ltmp_gdt_done - 0xC0000000
.Ltmp_gdt_done:
    mov $TMP_GDT_DATA_SEGMENT, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    movl $stack_top, %esp
    # Read current CR4 and set the PSE bit to enable 4MB pages
    mov %cr4, %ebx
    or $0x00000010, %ebx
    # Update CR4
    mov %ebx, %cr4
    lea page_directory - 0xC0000000, %eax
    mov %eax, %cr3
_enable_paging:
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0
    lea _higher_half, %eax
    jmp *%eax
_higher_half:
    # The identity mapping isn't needed anymore at this point
    movl $0, (page_directory)
    invlpg 0
    movl $stack_top, %esp
    # Now that we've enabled paging, call some C code to sets up the real GDT
    call init_gdt
    # Call the global constructors
    call _init
    # Set up the IDT and enable interrupts
    call init_idt
    call kernel_main
    # kernel_main should not return
    cli
halt:
    hlt
    jmp halt
