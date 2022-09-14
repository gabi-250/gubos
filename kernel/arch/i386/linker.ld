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

    .text.paging_bootstrap ALIGN(4K):
    {
	*(.text.paging_bootstrap)
    }

    .data.paging_bootstrap ALIGN(4K):
    {
	*(.data.paging_bootstrap)
    }

    /* Make sure the bootstrapping logic doesn't exceed 4MB! */
    /*. += 4M;*/

    __bootstrap_code_size = . - __kernel_physical_start;

    . = 0xC0000000;

    __higher_half = .;

    /* We want a higher-half kernel, so in the virtual address space, the
     * kernel lives at 3GB + 1MB
     *
     * NOTE: the virtual memory address (VMA) is set to 0xC0100000 by moving the
     * location counter. The load memory address (LMA) is specified using the
     * `AT` keyword.
     * */
    . = 0xC0100000;
    __kernel_virtual_start = .;
    __kernel_addr_space_delta = __kernel_virtual_start - __kernel_physical_start;

    /* Make sure we don't accidentally map back to the boostrap code. */
    . +=  __bootstrap_code_size;

    .text.boot.higher_half ALIGN(4K): AT(ADDR(.text.boot.higher_half) - __kernel_addr_space_delta)
    {
	*(.text.boot.higher_half)
    }

    .text ALIGN(4K): AT(ADDR(.text) - __kernel_addr_space_delta)
    {
	__kernel_text_virtual_start = .;
	__kernel_text_physical_start = ADDR(.text);
	*(.text)
    }

    /* Artificially pad the kernel to a size > 4MB (1 page) to test paging
     * boostrapping logic */
    /*. += 0x00400000;*/

    .rodata ALIGN(4K): AT(ADDR(.rodata) - __kernel_addr_space_delta)
    {
	__kernel_text_virtual_end = .;

	__kernel_rodata_virtual_start = .;
	__kernel_rodata_physical_start = ADDR(.rodata);
	*(.rodata)
    }

    .data ALIGN(4K): AT(ADDR(.data) - __kernel_addr_space_delta)
    {
	__kernel_rodata_virtual_end = .;

	__kernel_data_virtual_start = .;
	__kernel_data_physical_start = ADDR(.data);
	*(.data)
    }

    .bss ALIGN(4K): AT(ADDR(.bss) - __kernel_addr_space_delta)
    {
	__kernel_data_virtual_end = .;

	__kernel_bss_virtual_start = .;
	__kernel_bss_physical_start = ADDR(.bss);
	*(COMMON)
	*(.bss)
    }

    . = ALIGN(4K);
    __kernel_bss_virtual_end = .;

    /* Align the end of the kernel virtual address space to make it easily
     * mappable using 4MB pages */
    . = ALIGN(4M);
    __kernel_virtual_end = .;
    __kernel_physical_end = . - __kernel_addr_space_delta;
}
