#ifndef __VMM_H__
#define __VMM_H__

#include <stdint.h>
#include "mm/paging.h"

// A virtual allocation.
//
// This represents one or more mapped pages, starting at the specified virtual
// address. The allocated pages are not necessarily present in the page tables.
//
// NOTE: virtual_addr and physical_addr *must* be 4096 bytes aligned.
typedef struct vmm_allocation {
    uint32_t virtual_addr;
    uint32_t physical_addr;
    uint32_t page_count;
    uint32_t flags;
} vmm_allocation_t;

// The allocation search tree.
typedef struct vmm_allocation_tree {
    vmm_allocation_t *alloc;
    struct vmm_allocation_tree *left;
    struct vmm_allocation_tree *right;
    struct vmm_allocation_tree *parent;
} vmm_allocation_tree_t;

// A list of free (unmapped) areas in some virtual address space. The free area
// consists of page_count unmapped pages starting at a specific virtual address.
//
// NOTE: virtual_addr *must* be 4096 bytes aligned.
typedef struct vmm_free_blocks {
    uint32_t virtual_addr;
    uint32_t page_count;
    struct vmm_free_blocks *next;
} vmm_free_blocks_t;

typedef struct vmm_context {
    vmm_allocation_tree_t *allocations;
    vmm_free_blocks_t *free_blocks;
} vmm_context_t;

// Initialize the virtual memory manager.
void vmm_init();

// Allocate page_count consecutive pages starting at the specified virtual address.
//
// This maps the pages in the virtual address space of the current process,
// returning a pointer to the beginning of the newly allocated sequence of
// pages.
//
// The specified address *must* be 4096 bytes aligned.
void *vmm_map_pages(vmm_context_t *, uint32_t virtual_addr, uint32_t physical_addr,
                    uint32_t page_count, uint32_t flags);

// Free page_count consecutive pages starting at the specified page.
//
// The specified address *must* be 4096 bytes aligned.
void vmm_unmap_pages(vmm_context_t *, uint32_t virtual_addr, uint32_t page_count);

// Find the allocation that corresponds to the specified address.
vmm_allocation_t *vmm_find_allocation(vmm_context_t *, uint32_t virtual_addr);

// Map the specified virtual address to a physical address.
uint32_t vmm_virtual_to_physical(uint32_t addr);

// Map the specified physical address to a virtual address.
uint32_t vmm_physical_to_virtual(uint32_t addr);

#endif /* __VMM_H__ */
