#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "mm/paging.h"
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "printk.h"
#include "panic.h"

#define PAGE_SIZE KERNEL_PAGE_SIZE_4KB

extern kernel_meminfo_t KERNEL_MEMINFO;

page_table_t KERNEL_PAGE_TABLES[PAGE_TABLE_SIZE];
page_table_t ACTIVE_PAGE_DIRECTORY;

static inline uint32_t
virtual_to_physical(uint32_t addr) {
    // Subtract (virtual_start - physical_start) to get the physical address.
    uint32_t addr_space_delta = (KERNEL_MEMINFO.virtual_start - KERNEL_MEMINFO.physical_start);
    return addr - addr_space_delta;
}

static inline uint32_t
physical_to_virtual(uint32_t addr) {
    // Add (virtual_start - physical_start) to get the virtual address.
    uint32_t addr_space_delta = (KERNEL_MEMINFO.virtual_start - KERNEL_MEMINFO.physical_start);
    return addr + addr_space_delta;
}

void
init_paging() {
    memset(ACTIVE_PAGE_DIRECTORY.entries, sizeof(ACTIVE_PAGE_DIRECTORY.entries), 0);
    for (size_t i = 0; i < PAGE_TABLE_SIZE; ++i) {
        memset(KERNEL_PAGE_TABLES[i].entries, sizeof(KERNEL_PAGE_TABLES[i].entries), 0);
    }

    uint32_t higher_half_base = KERNEL_MEMINFO.higher_half_base;
    uint32_t virtual_end = KERNEL_MEMINFO.virtual_end;
    for (uint64_t i = 0; higher_half_base + i * KERNEL_PAGE_SIZE_4KB < virtual_end; ++i) {
        uint32_t virtual_addr = higher_half_base + i * KERNEL_PAGE_SIZE_4KB;
        uint32_t physical_addr = i * KERNEL_PAGE_SIZE_4KB;

        paging_map_virtual_to_physical(virtual_addr, physical_addr, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
    }

    uint32_t cr3 = virtual_to_physical((uint32_t)&ACTIVE_PAGE_DIRECTORY);
    // The last 4MB of virtual address space is reserved for bookkeeping: we map
    // the last page directory entry to the page directory itself (rather than
    // some other physical address), which makes it easy to access and modify
    // all the paging structures can (this is achieved by writing to a virtual
    // address which has all the 'directory' bits set to 1). This solves the
    // "chicken or the egg" problem that happens whenever the VMM creates a new
    // page to handle a mapping request, but the new page frame returned by the
    // PMM does not yet have a virtual mapping (so it can't be written to).
    //
    // TODO: implement what's described here.
    ACTIVE_PAGE_DIRECTORY.entries[PAGE_TABLE_SIZE - 1] = cr3 | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

    paging_set_page_directory(cr3);
}

// Load CR3 with the **physical** address of the page directory.
void
paging_set_page_directory(uint32_t addr) {
    asm volatile("mov %0, %%eax\n\t"
                 "mov %%eax, %%cr3" ::"r" (addr) : "memory");
}

void
paging_map_virtual_to_physical(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    page_table_t *page_table = &KERNEL_PAGE_TABLES[PAGE_DIRECTORY_INDEX(virtual_addr)];
    // page_table_addr is 4096 bytes aligned, so no need to clear the
    // lower 12 bits where the flags go
    uint32_t page_table_addr = virtual_to_physical((uint32_t)page_table);
    uint32_t page_start_addr = (physical_addr >> 12) << 12;

    ACTIVE_PAGE_DIRECTORY.entries[PAGE_DIRECTORY_INDEX(virtual_addr)] =
        page_table_addr | flags;
    page_table->entries[PAGE_TABLE_INDEX(virtual_addr)] =
        page_start_addr | flags;
}

inline bool
paging_is_aligned(uint32_t addr) {
    return !((addr) & (PAGE_SIZE - 1));
}

inline void
invlpg(uint32_t addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}
