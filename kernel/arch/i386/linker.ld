ENTRY(_start)

SECTIONS
{
    /* We want a higher-half kernel, which will live at 3GB + 1MB */
    . = 0xC0100000;

    __kernel_virtual_start = .;
    __kernel_physical_start = . - 0xC0000000;

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
