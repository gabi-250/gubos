#include <stdint.h>
#include <stddef.h>
#include "panic.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "multiboot2.h"
#include "kernel_meminfo.h"

// ((1 << 32) / (1 << 25))
// NOTE: each bit represents one 4M page.
#define MEM_BITMAP_SIZE (1 << 7)
#define BITMAP_ENTRY_MASK UINT8_MAX

static uint8_t MEM_BITMAP[MEM_BITMAP_SIZE];

static void
pmm_mark_addr_used(uint32_t addr) {
    size_t bitmap_bit = addr / PAGE_SIZE;
    size_t bitmap_index = bitmap_bit / 8;
    MEM_BITMAP[bitmap_index] |= (1 << (bitmap_bit % 8));
}

static void
pmm_mark_range_used(uint32_t start_addr, uint32_t end_addr) {
    for (uint32_t i = start_addr; i < end_addr; i += PAGE_SIZE) {
        pmm_mark_addr_used(i);
    }
}

static void
pmm_mark_addr_free(uint32_t addr) {
    size_t bitmap_bit = addr / PAGE_SIZE;
    size_t bitmap_index = bitmap_bit / 8;
    MEM_BITMAP[bitmap_index] &= ~(1 << bitmap_bit);
}

void
pmm_init(kernel_meminfo_t meminfo, multiboot_info_t multiboot_info) {
    // Mark the kernel physical address range as used:
    pmm_mark_range_used(meminfo.physical_start, meminfo.physical_end);
    // Mark any unavailable memory regions as used:
    struct multiboot_tag *tag = (struct multiboot_tag *) (multiboot_info.addr + 8);
    while (tag->type != MULTIBOOT_TAG_TYPE_END && tag->type != MULTIBOOT_TAG_TYPE_MMAP) {
        tag = (struct multiboot_tag *)((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7));
    }
    if (tag-> type == MULTIBOOT_TAG_TYPE_END) {
        PANIC("failed to read memory map");
    }
    multiboot_memory_map_t *mmap = ((struct multiboot_tag_mmap *) tag)->entries;
    while ((multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size) {
        switch (mmap->type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                break;
            case MULTIBOOT_MEMORY_RESERVED:
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            case MULTIBOOT_MEMORY_NVS:
            case MULTIBOOT_MEMORY_BADRAM:
                pmm_mark_range_used(mmap->addr, mmap->addr + mmap->len);
                break;
        }
        mmap = (multiboot_memory_map_t *)((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size);
    }
}

void *
pmm_alloc_page() {
    for (size_t i = 0; i < MEM_BITMAP_SIZE; ++i) {
        if (MEM_BITMAP[i] != BITMAP_ENTRY_MASK) {
            // This entry has at least one free slot
            size_t alloc_bit = 0;
            // Keep shifting until the first zeroed bit is found.
            for (uint8_t entry = MEM_BITMAP[i]; entry & 1; entry >>= 1, alloc_bit += 1);
            MEM_BITMAP[i] |= (1 << alloc_bit);
            return (void *)((i * 8 + alloc_bit) * PAGE_SIZE);
        }
    }
    // XXX handle this more gracefully
    PANIC("out of memory");
}

void
pmm_free_page(void *addr) {
    pmm_mark_addr_free((uint32_t)addr);
}
