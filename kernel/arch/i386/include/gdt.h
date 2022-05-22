#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>
#include <stdbool.h>

// These must be kept in sync with the gdt_init logic
#define GDT_KERNEL_CODE_SEGMENT         0x8
#define GDT_KERNEL_DATA_SEGMENT         0x10
#define GDT_USER_CODE_SEGMENT           0x18
#define GDT_USER_DATA_SEGMENT           0x20
#define GDT_TASK_STATE_SEGMENT          0x28

// The granularity flag
#define GDT_GRANULARITY_FLAG             0b1000
// The default operation size flag (0 = 16-bit segment, 1 = 32-bit segment).
#define GDT_DB_FLAG                      0b100
// The 64-bit code segment flag. If set, the segment is a 64-bit code segment.
#define GDT_L_FLAG                       0b10
// The segment present flag
#define GDT_SEGMENT_PRESENT_FLAG         0b10000000
// The code or data descriptor type (0 = system; 1 = code or data)
#define GDT_CODE_OR_DATA_DESCRIPTOR_FLAG 0b00010000
// The descriptor privilege level set to 3
#define GDT_DPL3_FLAG                    0b01100000

// ======================================================================
// Code- and Data-Segment Descriptor Types
// See Section 3.4.5.1 ("Code- and Data-Segment Descriptor Types") of the
// Intel® 64 and IA-32 Architectures Software Developer's Manual, Volume 3.
// ======================================================================

// An execute/read code segment type.
#define GDT_CODE_EXECUTE_READ_SEG_TYPE 0b1010
// An execute-only (accessed) code segment type.
#define GDT_CODE_EXECUTE_ONLY_ACCESSED 0b1001
// A read/write data segment type.
#define GDT_DATA_READ_WRITE_SEG_TYPE 0b0010

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
    //
    // | 7 | 6     5 | 4  | 3            0 |
    // | P | DPL     | S  | segment type   |
    uint8_t access_type;
    // | 7      4 | 3     0 |
    // | flags    | limit   |
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
    uint8_t flags;
    // Bits 24-31 of the segment base.
    uint8_t base_high;
} __attribute__((packed)) segment_descriptor_t;

typedef struct gdt_descriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_descriptor_t;

// See also: Figure 7-2 from Chapter 7 (Task Management) of
// Intel® 64 and IA-32 Architectures Software Developer's Manual, Volume 3.
typedef struct task_state_segment {
    uint16_t previous_task_link;
    uint16_t previous_task_link_reserved;
    // The stack pointer to use when switching from user mode to kernel mode.
    uint32_t esp0;
    // The stack segment to use when switching from user mode to kernel mode.
    uint16_t ss0;
    uint16_t ss0_reserved;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t ss1_reserved;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t ss2_reserved;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t es_reserved;
    uint16_t cs;
    uint16_t cs_reserved;
    uint16_t ss;
    uint16_t ss_reserved;
    uint16_t ds;
    uint16_t ds_reserved;
    uint16_t fs;
    uint16_t fs_reserved;
    uint16_t gs;
    uint16_t gs_reserved;
    uint16_t ldt_segment_selector;
    uint16_t ldt_segment_selector_reserved;
    // Bits 1-15 are reserved. Bit 0 is the T (debug trap) flag.
    uint16_t debug_trap;
    uint16_t io_map_base_address;
    uint32_t ssp;
} __attribute__((packed)) task_state_segment_t;

#endif /* __GDT_H__ */
