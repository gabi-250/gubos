#include <stdint.h>
#include <stdbool.h>
#include "interrupts/page_fault.h"
#include "interrupts/idt.h"
#include "printk.h"
#include "mm/vmm.h"

// Get the (linear) address that triggered the page fault.
static uint32_t
read_page_fault_addr() {
    // The CR2 register contains the 32-bit linear address that caused the page
    // fault.
    uint32_t addr = 0;
    asm volatile("mov %%cr2, %%eax\n\t"
                 "mov %%eax, %0\n\t"
                 ::"memory"(addr));
    return addr;
}

void
paging_handle_fault(interrupt_state_t * state, uint32_t err_code) {
    uint32_t addr = read_page_fault_addr();
    printk_debug("page fault @ %#x (access=%s, eflags=%#x, cs=%d, eip=%d)\n",
            addr,
            err_code & PAGING_ERR_CODE_WR ? "write": "read",
            state->eflags,
            state->cs,
            state->eip);
    // XXX map the address for now.
    vmm_map_addr((void *) addr, PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT);
}
