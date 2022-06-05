#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "mm/paging.h"
#include "printk.h"
#include "panic.h"
#include "kmalloc.h"
#include "kernel_meminfo.h"

#define PAGE_SIZE KERNEL_PAGE_SIZE_4KB

extern kernel_meminfo_t KERNEL_MEMINFO;

// The VMM state for the current task.
static vmm_context_t VMM_CONTEXT;

static void add_allocation(uint32_t virtual_addr, uint32_t page_count, uint32_t flags);
static void remove_allocation(vmm_allocation_tree_t *allocation);
static void add_free_blocks(vmm_free_blocks_t *free_blocks, uint32_t virtual_addr, uint32_t page_count);
static uint32_t remove_free_blocks(vmm_free_blocks_t *free_block, uint32_t virtual_addr, uint32_t page_count);
static vmm_allocation_t * find_allocation(vmm_allocation_tree_t *allocations, uint32_t virtual_addr);

void
vmm_init() {
    VMM_CONTEXT.allocations = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
    vmm_allocation_t alloc = {
        .virtual_addr = 0,
        .physical_addr = 0,
        .flags = 0,
    };

    // TODO traverse kernel meminfo and multiboot info to create the initial
    // allocations
    *VMM_CONTEXT.allocations = (vmm_allocation_tree_t){
        .alloc = alloc,
        .left = NULL,
        .right = NULL,
    };
}

void *
vmm_map_pages(uint32_t virtual_addr, uint32_t page_count, uint32_t flags) {
    // TODO handle flags
    if (!paging_is_aligned(virtual_addr)) {
        PANIC("cannot map unaligned address: %#x", virtual_addr);
    }

    if (!VMM_CONTEXT.free_blocks) {
        // XXX handle this more gracefully
        PANIC("Out of memory");
    }

    uint32_t addr = remove_free_blocks(VMM_CONTEXT.free_blocks, virtual_addr, page_count);
    add_allocation(addr, page_count, flags);

    return (void *)addr;
}

void
vmm_unmap_pages(uint32_t virtual_addr, uint32_t page_count) {
    if (!paging_is_aligned(virtual_addr)) {
        PANIC("cannot unmap unaligned address: %#x", virtual_addr);
    }

    // Update VMM_CONTEXT.free_blocks
    add_free_blocks(VMM_CONTEXT.free_blocks, virtual_addr, page_count);

    // Update VMM_CONTEXT.allocations
    vmm_allocation_tree_t *allocations = VMM_CONTEXT.allocations;

    while (allocations) {
        if (virtual_addr == allocations->alloc.virtual_addr) {
            if (page_count < allocations->alloc.virtual_addr) {
                // Shift the allocation
                allocations->alloc.virtual_addr += PAGE_SIZE * page_count;
                allocations->alloc.page_count -= page_count;
            } else if (page_count == allocations->alloc.virtual_addr) {
                // Delete the node alogether
                remove_allocation(allocations);
            } else {
                PANIC("cannot unamp more than has been mapped");
            }

            return;
        } else if (virtual_addr < allocations->alloc.virtual_addr) {
            allocations = allocations->left;
        } else {
            allocations = allocations->right;
        }
    }

    PANIC("allocation to unmap not found in allocation tree");
}

vmm_allocation_t *
vmm_find_allocation(uint32_t virtual_addr) {
    return find_allocation(VMM_CONTEXT.allocations, virtual_addr);
}

static void
remove_allocation(vmm_allocation_tree_t *allocation) {
    if (allocation->parent) {
        if (allocation->parent->left == allocation) {
            allocation->parent->left = allocation->left;
        } else {
            allocation->parent->right = allocation->left;
        }
    } else {
        // Remove the root
        VMM_CONTEXT.allocations = allocation->left;
    }

    vmm_allocation_tree_t * node = allocation->left;
    while (node->right) {
        node = node->right;
    }
    node->right = allocation->right;

    kfree(allocation);
}

static void
add_allocation(uint32_t virtual_addr, uint32_t page_count, uint32_t flags) {
    vmm_allocation_tree_t *allocations = VMM_CONTEXT.allocations;
    vmm_allocation_tree_t *prev = NULL;
    while (allocations) {
        prev = allocations;
        if (virtual_addr > allocations->alloc.virtual_addr) {
            allocations = allocations->right;
        } else {
            allocations = allocations->left;
        }
    }

    if (!prev) {
        PANIC("out of memory");
    }

    vmm_allocation_tree_t *new_node = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
    vmm_allocation_t alloc = {
        .virtual_addr = virtual_addr,
        .physical_addr = 0,
        .page_count = page_count,
        .flags = flags,
    };
    *new_node = (vmm_allocation_tree_t){
        .alloc = alloc,
        .left = NULL,
        .right = NULL,
    };

    new_node->parent = prev;
    if (virtual_addr > prev->alloc.virtual_addr) {
        prev->right = new_node;
    } else {
        prev->left = new_node;
    }
}

static uint32_t
remove_free_blocks(vmm_free_blocks_t *free_block, uint32_t virtual_addr, uint32_t page_count) {
    vmm_free_blocks_t *prev = NULL;

    while (free_block) {
        if (virtual_addr && free_block->virtual_addr != virtual_addr) {
            continue;
        }

        if (free_block->page_count > page_count) {
            uint32_t addr = free_block->virtual_addr;
            free_block->page_count -= page_count;
            free_block->virtual_addr += page_count * PAGE_SIZE;

            return addr;
        } else if (free_block->page_count == page_count) {
            uint32_t addr = free_block->virtual_addr;
            if (prev) {
                prev->next = free_block->next;
            } else {
                VMM_CONTEXT.free_blocks = free_block->next;
                kfree(free_block);
            }

            return addr;
        }

        prev = free_block;
        free_block = free_block->next;
    }

    PANIC("could not find %u consecutive free pages", page_count);
}

static void
merge_or_insert_free_blocks(vmm_free_blocks_t *free_blocks, uint32_t virtual_addr, uint32_t page_count) {
    bool blocks_merged = false;
    if (free_blocks) {
        // Merge the existing free blocks with the newly freed block if they
        // form a contiguous block.
        if (free_blocks->virtual_addr + page_count * PAGE_SIZE == virtual_addr) {
            free_blocks->page_count += page_count;
            blocks_merged = true;
        }

        if (free_blocks->next) {
            uint32_t next_free_addr = free_blocks->next->virtual_addr;

            if (free_blocks->virtual_addr + page_count * PAGE_SIZE == next_free_addr) {
                free_blocks->page_count += free_blocks->next->page_count;
                vmm_free_blocks_t *next = free_blocks->next;
                free_blocks->next = free_blocks->next->next;
                kfree(next);
                blocks_merged = true;
            }
        }
    }

    if (!blocks_merged) {
        vmm_free_blocks_t *new_block = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
        *new_block = (vmm_free_blocks_t){
            .virtual_addr = virtual_addr,
            .page_count = page_count,
            .next = free_blocks->next,
        };
        free_blocks->next = new_block;
    }
}

static void
add_free_blocks(vmm_free_blocks_t *free_blocks, uint32_t virtual_addr, uint32_t page_count) {
    vmm_free_blocks_t *prev = NULL;
    while (free_blocks && virtual_addr < free_blocks->virtual_addr) {
        prev = free_blocks;
        free_blocks = free_blocks->next;
    }

    if (free_blocks) {
        if (prev) {
            merge_or_insert_free_blocks(prev, virtual_addr, page_count);
        } else {
            if (virtual_addr + page_count * PAGE_SIZE == free_blocks->virtual_addr) {
                // Merge the existing free blocks with the newly freed block if they
                // form a contiguous block.
                free_blocks->page_count += page_count;
            } else {
                // The new block is the new head of the list
                VMM_CONTEXT.free_blocks = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
                *VMM_CONTEXT.free_blocks = (vmm_free_blocks_t){
                    .virtual_addr = virtual_addr,
                    .page_count = page_count,
                    .next = free_blocks,
                };
            }
        }
    } else {
        // virtual_addr is greater than any of the other free addresses.
        if (prev) {
            merge_or_insert_free_blocks(prev, virtual_addr, page_count);
        } else {
            // VMM_CONTEXT.free_blocks must've been NULL to begin with
            VMM_CONTEXT.free_blocks = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
            *VMM_CONTEXT.free_blocks = (vmm_free_blocks_t){
                .virtual_addr = virtual_addr,
                .page_count = page_count,
                .next = NULL,
            };
        }
    }
}

static vmm_allocation_t *
find_allocation(vmm_allocation_tree_t *allocations, uint32_t virtual_addr) {
    while (allocations) {
        uint32_t current_block = allocations->alloc.virtual_addr;
        uint32_t page_count = allocations->alloc.page_count;
        if (virtual_addr >= current_block && virtual_addr < current_block + page_count * PAGE_SIZE) {
            return &allocations->alloc;
        } else if (virtual_addr < current_block) {
            allocations = allocations->left;
        } else {
            allocations = allocations->right;
        }
    }
    // Not in tree:
    return NULL;
}
