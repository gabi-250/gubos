#include <stdint.h>
#include <interrupts/page_fault.h>
#include <printk.h>
#include <panic.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <task.h>

extern struct task_list CURRENT_TASK;

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
page_fault_handler(interrupt_state_t *state, uint32_t err_code) {
    uint32_t addr = read_page_fault_addr();
    printk_debug("[task %u] page fault @ %#x (eflags=%#x, cs=%d, eip=%d, err_code=%d): cause=%s, access=%s, mode=%s\n\t\n",
                 CURRENT_TASK.task->pid,
                 addr,
                 state->eflags,
                 state->cs,
                 state->eip,
                 err_code,
                 err_code & PAGING_ERR_CODE_P ? "protection_fault": "non_present_page",
                 err_code & PAGING_ERR_CODE_WR ? "write": "read",
                 err_code & PAGING_ERR_CODE_US ? "user": "kernel");

    if (err_code & PAGING_ERR_CODE_P) {
        // A protection fault is always an error
        if (err_code & PAGING_ERR_CODE_US) {
            PANIC("TODO: kill the misbehaving user process");
        } else {
            PANIC("kernel protection fault");
        }
    } else {
        uint32_t aligned_vaddr = paging_align_addr(addr);
        // Page not present
        vmm_allocation_t alloc = vmm_find_allocation(&CURRENT_TASK.task->vmm_context, aligned_vaddr);

        ASSERT(alloc.page_count, "invalid VMM state");

        uint32_t physical_addr = alloc.physical_addr ? alloc.physical_addr : (uint32_t)pmm_alloc_page();

        paging_map_virtual_to_physical(CURRENT_TASK.task->paging_ctx, aligned_vaddr, physical_addr,
                                       alloc.flags);
        printk_debug("mapped %#x -> %#x (flags=%u)\n", aligned_vaddr, physical_addr, alloc.flags);
    }
}
