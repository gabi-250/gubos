#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <panic.h>
#include <kmalloc.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <mm/meminfo.h>
#include <mm/addr_space.h>

extern kernel_meminfo_t KERNEL_MEMINFO;
extern multiboot_info_t MULTIBOOT_INFO;

static addr_space_entry_t ADDR_SPACE[3];

static void add_allocation(vmm_context_t *vmm_context, uint32_t virtual_addr,
                           uint32_t physical_addr, uint32_t page_count, uint32_t flags);
static void remove_allocation(vmm_context_t *vmm_context,
                              vmm_allocation_tree_t *allocation);
static void add_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                            uint32_t page_count);
static uint32_t remove_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                                   uint32_t page_count);
static vmm_context_t create_empty_ctx();

vmm_context_t
vmm_init() {
    // Kernel text, rodata, data
    ADDR_SPACE[0] = (addr_space_entry_t) {
        .virtual_start = KERNEL_MEMINFO.virtual_start,
        .physical_start = KERNEL_MEMINFO.physical_start,
        .page_count = (KERNEL_MEMINFO.virtual_end - KERNEL_MEMINFO.virtual_start) / PAGE_SIZE,
        .flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    };
    // Kernel heap
    ADDR_SPACE[1] = (addr_space_entry_t) {
        .virtual_start = KERNEL_HEAP_VIRT_START,
        .physical_start = KERNEL_HEAP_PHYS_START,
        .page_count = KERNEL_HEAP_SIZE / PAGE_SIZE,
        .flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    };
    // Framebuffer
    struct multiboot_tag_framebuffer_common *framebuffer_info =
        multiboot_framebuffer_info(MULTIBOOT_INFO.addr);
    ADDR_SPACE[2] = (addr_space_entry_t) {
        .virtual_start = KERNEL_MEMINFO.higher_half_base,
        .physical_start = framebuffer_info->framebuffer_addr,
        .page_count = 1,
        .flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    };

    return vmm_new_context();
}

vmm_context_t
vmm_new_context() {
    vmm_context_t vmm_context = create_empty_ctx();

    uint64_t total_page_count = ((uint64_t)1 << 32) / PAGE_SIZE;
    // Initially let's say the entire address space is available
    add_free_blocks(&vmm_context, 0, total_page_count);

    for (size_t i = 0; i < (sizeof(ADDR_SPACE) / sizeof(addr_space_entry_t)); ++i) {
        vmm_map_pages(&vmm_context,
                      ADDR_SPACE[i].virtual_start,
                      ADDR_SPACE[i].physical_start,
                      ADDR_SPACE[i].page_count,
                      ADDR_SPACE[i].flags);
    }

    return vmm_context;
}

vmm_free_blocks_t *
clone_free_blocks(vmm_free_blocks_t *orig_free_blocks) {
    if (!orig_free_blocks) {
        return NULL;
    }

    vmm_free_blocks_t *free_blocks = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
    vmm_free_blocks_t *prev = NULL;

    vmm_free_blocks_t *current = free_blocks;
    while (orig_free_blocks) {
        if (prev) {
            prev->next = current;
        }

        prev = current;

        *current = (vmm_free_blocks_t) {
            .virtual_addr = orig_free_blocks->virtual_addr,
            .page_count = orig_free_blocks->page_count,
            .next = NULL,
        };

        current = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
        orig_free_blocks = orig_free_blocks->next;
    }

    return free_blocks;
}

void
clone_allocation_tree(vmm_allocation_tree_t *orig_allocations, vmm_allocation_tree_t *allocations) {
    if (orig_allocations) {
        allocations->alloc = orig_allocations->alloc;

        if (orig_allocations->left) {
            allocations->left = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
            allocations->left->parent = allocations;
            clone_allocation_tree(orig_allocations->left, allocations->left);
        }

        if (orig_allocations->right) {
            allocations->right = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
            allocations->right->parent = allocations;
            clone_allocation_tree(orig_allocations->right, allocations->right);
        }
    }
}

vmm_allocation_tree_t *
clone_allocations(vmm_allocation_tree_t *orig_allocations) {
    if (!orig_allocations) {
        return NULL;
    }

    vmm_allocation_tree_t *allocations = (vmm_allocation_tree_t *)kmalloc(sizeof(
            vmm_allocation_tree_t));

    clone_allocation_tree(orig_allocations, allocations);

    return allocations;
}

vmm_context_t
vmm_clone_context(vmm_context_t orig_vmm_context) {
    return (vmm_context_t) {
        .free_blocks = clone_free_blocks(orig_vmm_context.free_blocks),
        .allocations = clone_allocations(orig_vmm_context.allocations)
    };
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

    uint32_t addr = remove_free_blocks(vmm_context, virtual_addr, page_count);
    add_allocation(vmm_context, addr, physical_addr, page_count, flags);

    return (void *)addr;
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
        if (virtual_addr == allocations->alloc.virtual_addr) {
            if (page_count < allocations->alloc.virtual_addr) {
                // Shift the allocation
                allocations->alloc.virtual_addr += PAGE_SIZE * page_count;
                allocations->alloc.page_count -= page_count;
            } else if (page_count == allocations->alloc.virtual_addr) {
                // Delete the node altogether
                remove_allocation(vmm_context, allocations);
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

vmm_allocation_t
vmm_find_allocation(vmm_context_t *vmm_context, uint32_t virtual_addr) {
    vmm_allocation_tree_t *allocations = vmm_context->allocations;

    while (allocations) {
        uint32_t current_block = allocations->alloc.virtual_addr;
        uint32_t page_count = allocations->alloc.page_count;
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
    return (vmm_allocation_t) {
        0
    };
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
    if (!allocations->alloc.page_count) {
        allocations->alloc = (vmm_allocation_t) {
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
        if (virtual_addr < allocations->alloc.virtual_addr) {
            allocations = allocations->left;
        } else {
            allocations = allocations->right;
        }
    }

    if (!prev) {
        PANIC("out of memory");
    }

    vmm_allocation_tree_t *new_node = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
    vmm_allocation_t alloc = (vmm_allocation_t) {
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
    if (virtual_addr < prev->alloc.virtual_addr) {
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

static uint32_t
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
            if (!virtual_addr || virtual_addr == free_blocks->virtual_addr) {
                uint32_t allocated_addr = free_blocks->virtual_addr;
                // The allocation is at the very beginning of the free region.
                free_blocks->virtual_addr += page_count * PAGE_SIZE;
                free_blocks->page_count -= page_count;

                return allocated_addr;
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

    return virtual_addr;
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

static vmm_context_t
create_empty_ctx() {
    vmm_context_t vmm_context;

    vmm_context.allocations = (vmm_allocation_tree_t *)kmalloc(sizeof(vmm_allocation_tree_t));
    *vmm_context.allocations = (vmm_allocation_tree_t) {
        .alloc = { 0 },
        .left = NULL,
        .right = NULL,
    };
    vmm_context.free_blocks = NULL;

    return vmm_context;
}
