#include <stdbool.h>
#include <stdint.h>
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "printk.h"

extern uint32_t page_directory;

static bool page_aligned(void *addr) {
    return !(((uint32_t)addr) & (PAGE_SIZE - 1));
}

/*static inline void invlpg(uint32_t addr) {*/
    /*asm volatile("invlpg (%0)" ::"r" (addr) : "memory");*/
/*}*/

void
vmm_map_addr(void *addr, uint32_t flags) {
    // The caller must ensure the address is page-aligned.
    if (!page_aligned(addr)) {
        printk_err("[VMM]: failed to map unaligned addr: %#x\n", addr);
        return;
    }

    uint32_t pdindex = ((uint32_t)addr) >> PAGE_SHIFT;
    void *physical_addr = pmm_alloc_page();
    (&page_directory)[pdindex] = ((uint32_t)physical_addr) | (flags & 0xFFF) | PAGE_FLAG_PAGE_SIZE | PAGE_FLAG_PRESENT;
    printk_debug("[VMM]: mapped %#x (virtual) -> %#x (physical)\n", addr, physical_addr);
}
