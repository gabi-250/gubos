#ifndef __ADDR_SPACE_H__
#define __ADDR_SPACE_H__

#include <mm/paging.h>
#define KERNEL_STACK_PAGE_COUNT 10
#define KERNEL_STACK_SIZE       KERNEL_STACK_PAGE_COUNT * PAGE_SIZE

#define USER_STACK_TOP          0x100000
#define USER_STACK_PAGE_COUNT   10
#define USER_STACK_SIZE         USER_STACK_PAGE_COUNT * PAGE_SIZE


#ifndef __ASSEMBLY__
#include <mm/vmm.h>

// Allocate a new kernel stack in the specified context and return the address
// of its top.
void *alloc_kernel_stack(vmm_context_t *vmm_context);
#endif

#endif /* __ADDR_SPACE_H__ */
