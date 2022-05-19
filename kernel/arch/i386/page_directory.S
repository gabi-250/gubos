#define __ASSEMBLY__ 1
#include "mm/vmm.h"

.set KERNEL_PAGE_TABLE_ENTRY, PAGE_DIRECTORY_INDEX(0xC0000000)

.globl page_directory

# The page tables
.align 4096
page_directory:
    # Entry 0 is for the identity mapping
    .long 0b10000011
.rept KERNEL_PAGE_TABLE_ENTRY - 1
    .long 0x00000000
.endr
    # The higher-half mapping
_higher_half_mapping:
    .long 0b10000011
.rept PAGE_TABLE_SIZE - KERNEL_PAGE_TABLE_ENTRY - 1
    .long 0x00000000
.endr

PAGE_TABLE_ENTRY_COUNT: .long PAGE_TABLE_SIZE
