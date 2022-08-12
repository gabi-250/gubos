#ifndef __ADDR_SPACE_H__
#define __ADDR_SPACE_H__

#include <mm/paging.h>
#define KERNEL_STACK_PAGE_COUNT 10
#define KERNEL_STACK_SIZE       KERNEL_STACK_PAGE_COUNT * PAGE_SIZE

#define USER_STACK_TOP          0xA0000000
#define USER_STACK_PAGE_COUNT   10
#define USER_STACK_SIZE         USER_STACK_PAGE_COUNT * PAGE_SIZE


#ifndef __ASSEMBLY__
#include <mm/vmm.h>

typedef struct addr_space_entry {
    uint32_t virtual_start;
    uint32_t physical_start;
    uint32_t page_count;
    uint32_t flags;
} addr_space_entry_t;

// Allocate a new kernel stack in the specified context and return the address
// of its top.
void *alloc_kernel_stack(paging_context_t, vmm_context_t *);
#endif

#endif /* __ADDR_SPACE_H__ */
