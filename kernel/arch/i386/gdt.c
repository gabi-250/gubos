#include <stdint.h>
#include "gdt.h"

#define GDT_ENTRY_COUNT 5

extern void load_gdt(uint32_t, uint16_t, uint16_t);

typedef struct segment_descriptor {
    // Bits 0-15 of the segment limit.
    uint16_t limit_low;
    // Bits 0-15 of the segment base.
    uint16_t base_low;
    // Bits 16-23 of the segment base.
    uint8_t base_middle;
    // Specifies the kind of access that can be made to the segment.
    //
    // See Section 3.4.5.1 ("Code- and Data-Segment Descriptor Types") of the
    // Intel manual for more details.
    //
    // For example, 0b0010 should be used for a Read/Write data descriptor,
    // while 0b1010 can be used for an Execute/Read code descriptor.
    uint8_t access_type;
    // | 3      0 | 19     16 |
    // | flags    | limit     |
    //
    // where "limit" contains bits 16-19 of the segment limit, and the flags are:
    // | 3 |  2  |  1 |  0  |
    // | G | D/B | L  | AVL |
    //
    // G   - the granularity flag
    // D/B - the default operation size flag (0 = 16-bit segment, 1 = 32-bit segment)
    // L   - the 64-bit code segment flag. If set, the segment is a 64-bit code segment
    // AVL - "available for use by system software", so in other words, reserved
    //       and should be set to 0
    uint8_t granularity;
    // Bits 24-31 of the segment base.
    uint8_t base_high;
} __attribute__((packed)) segment_descriptor_t;

typedef struct gdt_descriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_descriptor_t;

static segment_descriptor_t gdt_entries[GDT_ENTRY_COUNT];

// A pointer to the 48-bit GDT structure (this needs to be passed to lgdt)
static gdt_descriptor_t gdt;

static void
set_segment_descriptor(uint32_t i, uint32_t base, uint32_t limit,
        uint8_t access_type)
{
    // The lower 16 bits of base:
    gdt_entries[i].base_low = base & 0xffff;
    // The next 8 bits of the base:
    gdt_entries[i].base_middle = (base >> 16) & 0xff;
    // The 8 most significant bits of the base:
    gdt_entries[i].base_high = base >> 24;
    gdt_entries[i].access_type = access_type;
    // The lower 16 bits of limit:
    gdt_entries[i].limit_low = limit & 0xffff;
    gdt_entries[i].granularity = (limit >> 16) & 0x0f;
    // Set these flags (0b1100):
    // G   = 1
    // D/B = 1
    // L   = 0
    // AVL = 0
    gdt_entries[i].granularity |= 0b1100 << 4;
}

void
init_gdt() {
    // The NULL segment
    set_segment_descriptor(0, 0, 0, 0);
    // The kernel code descriptor
    set_segment_descriptor(1, 0, 0xFFFFFFFF, 0x9A);
    // The kernel data descriptor
    set_segment_descriptor(2, 0, 0xFFFFFFFF, 0x92);
    // The user-mode code descriptor
    set_segment_descriptor(3, 0, 0xFFFFFFFF, 0xFA);
    // The user-mode data descriptor
    set_segment_descriptor(4, 0, 0xFFFFFFFF, 0xF2);
    gdt.base = (uint32_t)gdt_entries;
    gdt.limit = (sizeof(segment_descriptor_t) * GDT_ENTRY_COUNT) - 1;
    load_gdt((uint32_t)&gdt, GDT_KERNEL_CODE_SEGMENT, GDT_KERNEL_DATA_SEGMENT);
}
