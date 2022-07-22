#include <mm/addr_space.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>

#include <stddef.h>

void *
alloc_kernel_stack(paging_context_t paging_ctx, vmm_context_t *vmm_context) {
    uint32_t kernel_stack_bottom = (uint32_t)vmm_map_pages(vmm_context, 0, 0, KERNEL_STACK_PAGE_COUNT,
                                   PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
    // Make sure the stack top is within the allocated region and 16-bytes
    // aligned (the call instruction has this alignment requirement).
    uint32_t kernel_stack_top = kernel_stack_bottom + KERNEL_STACK_SIZE - 16;

    // If the kernel stack is not mapped, you're going to have a bad time.
    for (size_t i = 0; i < KERNEL_STACK_PAGE_COUNT; ++i) {
        uint32_t physical_addr = (uint32_t)pmm_alloc_page();
        paging_map_virtual_to_physical(paging_ctx, kernel_stack_bottom + i * PAGE_SIZE, physical_addr,
                                       PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
    }

    return (void *)kernel_stack_top;
}
