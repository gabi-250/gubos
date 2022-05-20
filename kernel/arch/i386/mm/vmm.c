#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "printk.h"

static bool page_aligned(void *addr) {
    return !(((uint32_t)addr) & (KERNEL_PAGE_SIZE - 1));
}

extern kernel_meminfo_t PMM_KERNEL_MEMINFO;
extern page_table_t KERNEL_PAGE_DIRECTORY;

// Invalidate the TLB entries of the page that corresponds to the specified
// address.
static inline void
invlpg(uint32_t addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

static inline uint32_t
virtual_to_physical(uint32_t addr) {
    // Subtract (virtual_start - physical_start) to get the physical address.
    uint32_t addr_space_delta = (PMM_KERNEL_MEMINFO.virtual_start - PMM_KERNEL_MEMINFO.physical_start);
    return addr - addr_space_delta;
}

static inline uint32_t
physical_to_virtual(uint32_t addr) {
    // Add (virtual_start - physical_start) to get the virtual address.
    uint32_t addr_space_delta = (PMM_KERNEL_MEMINFO.virtual_start - PMM_KERNEL_MEMINFO.physical_start);
    return addr + addr_space_delta;
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
        asm volatile("hlt");
        return;
    }

    uint32_t pd_index = PAGE_DIRECTORY_INDEX((uint32_t)addr);
    uint32_t physical_addr = (uint32_t)pmm_alloc_page();

    // KERNEL_PAGE_DIRECTORY was linked in the lower half and at this point
    // the identity mapping for the lower half no longer exists, so we must work
    // out the higher-half virtual address of the page directory.
    page_table_t *kernel_page_directory =
        (page_table_t *)physical_to_virtual((uint32_t)&KERNEL_PAGE_DIRECTORY);

    kernel_page_directory->entries[pd_index] =
        physical_addr | (flags & 0xFFF) | PAGE_FLAG_PAGE_SIZE | PAGE_FLAG_PRESENT;

    printk_debug("[VMM]: mapped %#x (virtual) -> %#x (physical)\n", addr, physical_addr);
}
