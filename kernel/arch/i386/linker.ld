ENTRY(_start)

SECTIONS
{
    /* The kernel is loaded at 1 MB in the physical address space... */
    . = 0x00100000;

    __kernel_physical_start = .;

    .text.boot ALIGN(4K):
    {
	/* Keep the multiboot section even if no symbols inside it are referenced */
	KEEP(*(.multiboot))
	*(.text.boot)
    }

    .data.boot ALIGN(4K):
    {
	*(.data.boot)
    }

    __bootstrap_code_size = . - __kernel_physical_start;

    /* We want a higher-half kernel, so in the virtual address space, the
     * kernel lives at 3GB + 1MB
     *
     * NOTE: the virtual memory address (VMA) is set to 0xC0100000 by moving the
     * location counter. The load memory address (LMA) is specified using the
     * `AT` keyword.
     * */
    . = 0xC0100000;
    __kernel_virtual_start = .;

    /* Make sure we don't accidentally map back to the boostrap code. */
    . +=  __bootstrap_code_size;

    .text.boot.higher_half ALIGN(4K): AT(ADDR(.text.boot.higher_half) - 0xC0000000)
    {
	*(.text.boot.higher_half)
    }

    .text ALIGN(4K): AT(ADDR(.text) - 0xC0000000)
    {
	*(.text)
    }

    /* Artificially pad the kernel to a size > 4MB (1 page) to test paging
     * boostrapping logic */
    /*. += 0x00400000;*/

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
