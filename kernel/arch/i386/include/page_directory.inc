.set PAGE_TABLE_ENTRIES, 1024
.set KERNEL_PAGE_TABLE_ENTRY, 0xC0000000 >> 22

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
.rept PAGE_TABLE_ENTRIES - KERNEL_PAGE_TABLE_ENTRY - 1
    .long 0x00000000
.endr
