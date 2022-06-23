#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <panic.h>
#include <kmalloc.h>
#include <mm/meminfo.h>

#define PAGE_SIZE KERNEL_PAGE_SIZE_4KB

extern kernel_meminfo_t KERNEL_MEMINFO;
extern multiboot_info_t MULTIBOOT_INFO;

// The VMM state for the current task.
vmm_context_t VMM_CONTEXT;

static void add_allocation(vmm_context_t *vmm_context, uint32_t virtual_addr,
                           uint32_t physical_addr, uint32_t page_count, uint32_t flags);
static void remove_allocation(vmm_context_t *vmm_context,
                              vmm_allocation_tree_t *allocation);
static void add_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                            uint32_t page_count);
static void remove_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                               uint32_t page_count);

void
vmm_init() {
    VMM_CONTEXT.allocations = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
    *VMM_CONTEXT.allocations = (vmm_allocation_tree_t) {
        .alloc = NULL,
        .left = NULL,
        .right = NULL,
    };
    // TODO populate with free blocks and allocations before calling
    // vmm_map_pages
    VMM_CONTEXT.free_blocks = NULL;

    uint64_t total_page_count = ((uint64_t)1 << 32) / KERNEL_PAGE_SIZE_4KB;
    // Initially let's say the entire address space is available
    add_free_blocks(&VMM_CONTEXT, 0, total_page_count);

    vmm_map_pages(&VMM_CONTEXT, KERNEL_HEAP_START, 0, KERNEL_HEAP_SIZE / KERNEL_PAGE_SIZE_4KB,
                  PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);

    // Map the framebuffer
    struct multiboot_tag_framebuffer_common *framebuffer_info =
        multiboot_framebuffer_info(MULTIBOOT_INFO.addr);

    vmm_map_pages(&VMM_CONTEXT, KERNEL_MEMINFO.higher_half_base, framebuffer_info->framebuffer_addr, 1,
                  PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);

    // Map the kernel
    uint32_t page_count = (KERNEL_MEMINFO.virtual_end - KERNEL_MEMINFO.virtual_start) /
                          KERNEL_PAGE_SIZE_4KB;
    vmm_map_pages(&VMM_CONTEXT, KERNEL_MEMINFO.virtual_start,
                  KERNEL_MEMINFO.physical_start, page_count,
                  PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
}

void *
vmm_map_pages(vmm_context_t *vmm_context, uint32_t virtual_addr, uint32_t physical_addr,
              uint32_t page_count, uint32_t flags) {
    // TODO handle flags
    ASSERT(paging_is_aligned(virtual_addr), "cannot map unaligned address: %#x", virtual_addr);

    if (!vmm_context->free_blocks) {
        // XXX handle this more gracefully
        PANIC("Out of memory");
    }

    remove_free_blocks(vmm_context, virtual_addr, page_count);
    add_allocation(vmm_context, virtual_addr, physical_addr, page_count, flags);

    return (void *)virtual_addr;
}

void
vmm_unmap_pages(vmm_context_t *vmm_context, uint32_t virtual_addr, uint32_t page_count) {
    ASSERT(paging_is_aligned(virtual_addr),
           "cannot unmap unaligned address: %#x", virtual_addr);

    // Update vmm_context->free_blocks
    add_free_blocks(vmm_context, virtual_addr, page_count);

    // Update vmm_context->allocations
    vmm_allocation_tree_t *allocations = vmm_context->allocations;

    while (allocations) {
        if (virtual_addr == allocations->alloc->virtual_addr) {
            if (page_count < allocations->alloc->virtual_addr) {
                // Shift the allocation
                allocations->alloc->virtual_addr += PAGE_SIZE * page_count;
                allocations->alloc->page_count -= page_count;
            } else if (page_count == allocations->alloc->virtual_addr) {
                // Delete the node altogether
                remove_allocation(vmm_context, allocations);
            } else {
                PANIC("cannot unamp more than has been mapped");
            }

            return;
        } else if (virtual_addr < allocations->alloc->virtual_addr) {
            allocations = allocations->left;
        } else {
            allocations = allocations->right;
        }
    }

    PANIC("allocation to unmap not found in allocation tree");
}

vmm_allocation_t *
vmm_find_allocation(vmm_context_t *vmm_context, uint32_t virtual_addr) {
    vmm_allocation_tree_t *allocations = vmm_context->allocations;

    while (allocations) {
        uint32_t current_block = allocations->alloc->virtual_addr;
        uint32_t page_count = allocations->alloc->page_count;
        if (virtual_addr >= current_block
                && virtual_addr < current_block + page_count * PAGE_SIZE) {
            return allocations->alloc;
        } else if (virtual_addr < current_block) {
            allocations = allocations->left;
        } else {
            allocations = allocations->right;
        }
    }
    // Not in tree:
    return NULL;
}

inline uint32_t
vmm_virtual_to_physical(uint32_t addr) {
    // Subtract (virtual_start - physical_start) to get the physical address.
    return addr - KERNEL_MEMINFO.higher_half_base;
}

inline uint32_t
vmm_physical_to_virtual(uint32_t addr) {
    // Add (virtual_start - physical_start) to get the virtual address.
    return addr + KERNEL_MEMINFO.higher_half_base;
}

static void
remove_allocation(vmm_context_t *vmm_context, vmm_allocation_tree_t *allocation) {
    if (allocation->parent) {
        if (allocation->parent->left == allocation) {
            allocation->parent->left = allocation->left;
        } else {
            allocation->parent->right = allocation->left;
        }
    } else {
        // Remove the root
        vmm_context->allocations = allocation->left;
    }

    vmm_allocation_tree_t *node = allocation->left;
    while (node->right) {
        node = node->right;
    }
    node->right = allocation->right;

    kfree(allocation);
}

static void
add_allocation(vmm_context_t *vmm_context, uint32_t virtual_addr, uint32_t physical_addr,
               uint32_t page_count, uint32_t flags) {
    vmm_allocation_tree_t *allocations = vmm_context->allocations;

    // Empty allocations tree
    if (!allocations->alloc) {
        allocations->alloc = (vmm_allocation_t *)kmalloc(sizeof(vmm_allocation_t));
        *allocations->alloc = (vmm_allocation_t) {
            virtual_addr = virtual_addr,
            physical_addr = physical_addr,
            page_count = page_count,
            flags = flags,
        };

        return;
    }

    vmm_allocation_tree_t *prev = NULL;
    while (allocations) {
        prev = allocations;
        if (virtual_addr < allocations->alloc->virtual_addr) {
            allocations = allocations->left;
        } else {
            allocations = allocations->right;
        }
    }

    if (!prev) {
        PANIC("out of memory");
    }

    vmm_allocation_tree_t *new_node = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
    vmm_allocation_t *alloc= (vmm_allocation_t *)kmalloc(sizeof(vmm_allocation_t));
    *alloc = (vmm_allocation_t) {
        .virtual_addr = virtual_addr,
        .physical_addr = physical_addr,
        .page_count = page_count,
        .flags = flags,
    };
    *new_node = (vmm_allocation_tree_t) {
        .alloc = alloc,
        .left = NULL,
        .right = NULL,
    };

    new_node->parent = prev;
    if (virtual_addr < prev->alloc->virtual_addr) {
        prev->left = new_node;
    } else {
        prev->right = new_node;
    }
}

static bool
is_addr_in_range(uint32_t virtual_addr, vmm_free_blocks_t *free_blocks) {
    uint64_t range_end = free_blocks->virtual_addr + (uint64_t)free_blocks->page_count * PAGE_SIZE;

    return virtual_addr >= free_blocks->virtual_addr && virtual_addr < range_end;
}

static void
unlink_block(vmm_context_t *vmm_context, vmm_free_blocks_t *free_blocks, vmm_free_blocks_t *prev) {
    if (prev) {
        prev->next = free_blocks->next;
    } else {
        vmm_context->free_blocks = free_blocks->next;
    }
    kfree(free_blocks);
}

static void
remove_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                   uint32_t page_count) {
    ASSERT(vmm_context->free_blocks, "no free blocks");

    vmm_free_blocks_t *free_blocks = vmm_context->free_blocks;
    vmm_free_blocks_t *prev = NULL;

    while (free_blocks) {
        // If the caller requested a _specific_ virtual address, try to find it.
        if (!virtual_addr || is_addr_in_range(virtual_addr, free_blocks)) {

            if (free_blocks->page_count >= page_count) {
                break;
            }
        }
        prev = free_blocks;
        free_blocks = free_blocks->next;
    }

    if (!free_blocks) {
        PANIC("could not find %u consecutive free pages starting at %#x", page_count, virtual_addr);
    }

    // Maybe the requested allocation fits perfectly in `free_blocks`.
    if (free_blocks->page_count == page_count) {
        unlink_block(vmm_context, free_blocks, prev);
    } else if (free_blocks->page_count > page_count) {
        // The requested virtual address is `page_offset` pages into the free
        // region.
        uint32_t page_offset = (virtual_addr - free_blocks->virtual_addr) / PAGE_SIZE;

        // Are there enough pages to satisfy the request?
        if (page_count > free_blocks->page_count - page_offset) {
            PANIC("out of memory");
        } else {
            if (virtual_addr == free_blocks->virtual_addr) {
                // The allocation is at the very beginning of the free region.
                free_blocks->virtual_addr += page_count * PAGE_SIZE;
                free_blocks->page_count -= page_count;
            } else {
                uint32_t new_block_page_offset = page_offset + page_count;

                // Is the requested virtual address somewhere in the middle of the
                // free region rather than at its beginning/end?
                if (new_block_page_offset < free_blocks->page_count) {
                    vmm_free_blocks_t *new_block = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
                    *new_block = (vmm_free_blocks_t) {
                        // The remaining free region immediately follows the
                        // one starting at `virtual_addr`.
                        .virtual_addr = free_blocks->virtual_addr + new_block_page_offset * PAGE_SIZE,
                        .page_count = free_blocks->page_count - new_block_page_offset,
                        .next = free_blocks->next,
                    };
                    free_blocks->next = new_block;
                    free_blocks->page_count = page_offset;
                } else {
                    // ..no, it's at the end of the region:
                    free_blocks->page_count -= page_count;
                }
            }
        }
    }
}

static void
merge_or_insert_free_blocks(vmm_free_blocks_t *free_blocks, uint32_t virtual_addr,
                            uint32_t page_count) {
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
        *new_block = (vmm_free_blocks_t) {
            .virtual_addr = virtual_addr,
            .page_count = page_count,
            .next = free_blocks->next,
        };
        free_blocks->next = new_block;
    }
}

static void
add_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr, uint32_t page_count) {
    vmm_free_blocks_t *free_blocks = vmm_context->free_blocks;
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
                vmm_context->free_blocks = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
                *vmm_context->free_blocks = (vmm_free_blocks_t) {
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
            // vmm_context->free_blocks must've been NULL to begin with
            vmm_context->free_blocks = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
            *vmm_context->free_blocks = (vmm_free_blocks_t) {
                .virtual_addr = virtual_addr,
                .page_count = page_count,
                .next = NULL,
            };
        }
    }
}
