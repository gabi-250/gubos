tmp_gdt:
null_descriptor:
    .long 0x0
    .long 0x0
code_descriptor:
    # Base = 0x0 and limit = 0xfffff
    # Limit
    .word 0xffff
    # Base (bits 0-15)
    .word 0x0
    # Base (bits 16-23)
    .byte 0x0
    .byte 0b10011010
    .byte 0b11001111
    # Base (bits 24-31)
    .byte 0x0
data_descriptor:
    # Base = 0x0 and limit = 0xfffff (overlaps with the code descriptor)
    # Limit
    .word 0xffff
    # Base (bits 0-15)
    .word 0x0
    # Base (bits 16-23)
    .byte 0x0
    .byte 0b10010010
    .byte 0b11001111
    # Base (bits 24-31)
    .byte 0x0
tmp_gdt_descriptor:
tmp_gdt_max_offset: .word (. - tmp_gdt - 1)
tmp_gdt_start:      .long tmp_gdt - 0xC0000000

.set GDT_CODE_SEGMENT, (code_descriptor - tmp_gdt)
.set GDT_DATA_SEGMENT, (data_descriptor - tmp_gdt)
