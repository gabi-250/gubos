ENTRY(_start)

SECTIONS
{
    /* The kernel is loaded at 1 MB in the physical address space... */
    . = 0x00100000;

    __kernel_physical_start = .;

    /* ...but we want a higher-half kernel, so in the virtual address space, the
     * kernel lives at 3GB + 1MB
     *
     * NOTE: the virtual memory address (VMA) is set to 0xC0100000 by moving the
     * location counter. The load memory address (LMA) is specified using the
     * `AT` keyword.
     * */
    . += 0xC0000000;
    __kernel_virtual_start = .;

    .text ALIGN(4K): AT(ADDR(.text) - 0xC0000000)
    {
	/* Keep the multiboot section at the beginning of the text segment */
	KEEP(*(.multiboot))
	*(.text)
    }

    .rodata ALIGN(4K): AT(ADDR(.rodata) - 0xC0000000)
    {
	*(.rodata)
    }

    .data ALIGN(4K): AT(ADDR(.data) - 0xC0000000)
    {
	*(.data)
    }

    .bss ALIGN(4K): AT(ADDR(.bss) - 0xC0000000)
    {
	*(COMMON)
	*(.bss)
    }

    __kernel_virtual_end = .;
    __kernel_physical_end = . - 0xC0000000;
}
