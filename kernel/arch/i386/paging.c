#include <stdint.h>
#include "paging.h"
#include "idt.h"
#include "printk.h"

void
paging_handle_fault(interrupt_state_t * state, unsigned int err_code) {
    // The CR2 register contains the 32-bit linear address that caused the page
    // fault.
    uint32_t addr = 0;
    asm volatile("mov %%cr2, %%eax\n\t"
                 "mov %%eax, %0\n\t"
                 ::"memory"(addr));
    printk_warn("page fault @ %#x (access=%s, eflags=%#x, cs=%d, eip=%d)\n",
            addr,
            err_code & PAGING_ERR_CODE_WR ? "write": "read",
            state->eflags,
            state->cs,
            state->eip);
}
