#include <stdint.h>
#include "kernel_meminfo.h"
#include "gdt.h"
#include "string.h"

#define GDT_ENTRY_COUNT 6

extern void load_gdt(uint32_t, uint16_t, uint16_t);
extern void load_tss(uint32_t);

static segment_descriptor_t gdt_entries[GDT_ENTRY_COUNT];

// A pointer to the 48-bit GDT structure (this needs to be passed to lgdt)
static gdt_descriptor_t gdt;

// The task-state segment.
task_state_segment_t tss;

static void
tss_init(kernel_meminfo_t meminfo) {
    memset(&tss, sizeof(tss), 0);
    tss.ss0 = GDT_KERNEL_DATA_SEGMENT;
    tss.esp0 = meminfo.stack_top; // TODO pick a stack address
    // XXX what about io_map_base_address?
}

static void
set_segment_descriptor(uint32_t i, uint32_t base, uint32_t limit, uint8_t access_type, uint8_t flags)
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
    gdt_entries[i].flags = (limit >> 16) & 0x0f;
    gdt_entries[i].flags |= flags << 4;
}

void
gdt_init(kernel_meminfo_t meminfo) {
    // NOTE: TSS is used to build the kernel stack to use for interrupt handling
    // (whenever an interrupt occurs in userspace, the CPU needs to be able to
    // prepare the kernel stack before switching to ring 0 for interrupt handling).
    tss_init(meminfo);
    // The NULL segment
    set_segment_descriptor(0, 0, 0, 0, 0);

    // The kernel code descriptor
    uint8_t kernel_code_desc_flags =
        GDT_SEGMENT_PRESENT_FLAG | GDT_CODE_OR_DATA_DESCRIPTOR_FLAG | GDT_CODE_EXECUTE_READ_SEG_TYPE;
    set_segment_descriptor(1, 0, 0xFFFFFFFF, kernel_code_desc_flags, GDT_GRANULARITY_FLAG | GDT_DB_FLAG);

    // The kernel data descriptor
    uint8_t kernel_data_desc_flags =
        GDT_SEGMENT_PRESENT_FLAG | GDT_CODE_OR_DATA_DESCRIPTOR_FLAG | GDT_DATA_READ_WRITE_SEG_TYPE;
    set_segment_descriptor(2, 0, 0xFFFFFFFF, kernel_data_desc_flags, GDT_GRANULARITY_FLAG | GDT_DB_FLAG);

    // The user-mode code descriptor
    set_segment_descriptor(3, 0, 0xFFFFFFFF, kernel_code_desc_flags | GDT_DPL3_FLAG, GDT_GRANULARITY_FLAG | GDT_DB_FLAG);

    // The user-mode data descriptor
    set_segment_descriptor(4, 0, 0xFFFFFFFF, kernel_data_desc_flags | GDT_DPL3_FLAG, GDT_GRANULARITY_FLAG | GDT_DB_FLAG);

    // The TSS descriptor
    uint8_t tss_desc_flags = GDT_SEGMENT_PRESENT_FLAG | GDT_CODE_EXECUTE_ONLY_ACCESSED;
    set_segment_descriptor(5, (uint32_t)&tss, sizeof(task_state_segment_t) - 1, tss_desc_flags, 0x0);
    gdt.base = (uint32_t)gdt_entries;
    gdt.limit = (sizeof(segment_descriptor_t) * GDT_ENTRY_COUNT) - 1;
    load_gdt((uint32_t)&gdt, GDT_KERNEL_CODE_SEGMENT, GDT_KERNEL_DATA_SEGMENT);

    load_tss(GDT_TASK_STATE_SEGMENT);
}
