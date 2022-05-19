#include <stdbool.h>
#include <stdint.h>
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "kernel_meminfo.h"
#include "printk.h"
#include "string.h"

extern uint32_t page_directory;

typedef struct page_directory {
    uint32_t entries[PAGE_TABLE_SIZE];
} __attribute__ ((aligned(4096))) page_directory_t;

typedef struct page_table {
    uint32_t entries[PAGE_TABLE_SIZE];
} __attribute__ ((aligned(4096))) page_table_t;

// NOTE: The kernel uses 4MB pages, so we only really need to use 256 out of the
// 1024 available entries (256 * 4MB = 1GB)
page_table_t KERNEL_PAGE_DIRECTORY;

static bool page_aligned(void *addr) {
    return !(((uint32_t)addr) & (KERNEL_PAGE_SIZE - 1));
}

// Invalidate the TLB entries of the page that corresponds to the specified
// address.
static inline void
invlpg(uint32_t addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

// Create a page directory entry (4MB pages) from the specified physical address
//
// The format of a page directory entry (with 4MB pages) is:
// | 31                                    22| 21        17 | 16                 13    | 12  | 11    0 |
// |---------------------------------------------------------------------------------------------------|
// | bits 31:22 of address of 4MB page frame | 0 (reserved) | bits 39:32 of address[2] | PAT | flags   |
//
// [2] This example illustrates a processor in which MAXPHYADDR is 36. If this
// value is larger or smaller, the number of bits reserved in positions 20:13 of
// a PDE mapping a 4-MByte page will change.
static inline uint32_t
physical_addr_to_pde(uint32_t addr) {
    return (addr >> 22) << 22;
}

void
vmm_init(kernel_meminfo_t meminfo) {
    memset(KERNEL_PAGE_DIRECTORY.entries, PAGE_TABLE_SIZE, 0);

    for (uint64_t i = 0; meminfo.virtual_start + i * KERNEL_PAGE_SIZE <= KERNEL_VIRTUAL_END; ++i) {
        uint32_t virtual_address = meminfo.virtual_start + i * KERNEL_PAGE_SIZE;
        uint32_t physical_address = meminfo.physical_start + i * KERNEL_PAGE_SIZE;
        uint32_t entry = physical_addr_to_pde(physical_address);

        KERNEL_PAGE_DIRECTORY.entries[PAGE_DIRECTORY_INDEX(virtual_address)] =
            entry | PAGE_FLAG_PRESENT | PAGE_FLAG_PAGE_SIZE | PAGE_FLAG_WRITE;
    }

    // Subtract (virtual_start - physical_start) to get the physical address of
    // the page directory.
    uint32_t addr_space_delta = (meminfo.virtual_start - meminfo.physical_start);
    uint32_t kernel_page_directory = (uint32_t)&KERNEL_PAGE_DIRECTORY - addr_space_delta;
    vmm_set_page_directory(kernel_page_directory);
}

// Load CR3 with the **physical** address of the page directory.
void
vmm_set_page_directory(uint32_t addr) {
    asm volatile("mov %0, %%eax\n\t"
                 "mov %%eax, %%cr3" ::"r" (addr) : "memory");
}

void
vmm_map_addr(void *addr, uint32_t flags) {
    // The caller must ensure the address is page-aligned.
    if (!page_aligned(addr)) {
        printk_err("[VMM]: failed to map unaligned addr: %#x\n", addr);
        return;
    }

    uint32_t pd_index = PAGE_DIRECTORY_INDEX((uint32_t)addr);
    uint32_t physical_addr = (uint32_t)pmm_alloc_page();

    KERNEL_PAGE_DIRECTORY.entries[pd_index]
        = physical_addr | (flags & 0xFFF) | PAGE_FLAG_PAGE_SIZE | PAGE_FLAG_PRESENT;

    printk_debug("[VMM]: mapped %#x (virtual) -> %#x (physical)\n", addr, physical_addr);
}
