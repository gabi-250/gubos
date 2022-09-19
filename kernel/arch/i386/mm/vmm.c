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
#include <panic.h>

#define ADDR_SPACE_ENTRIES 6

extern kernel_meminfo_t KERNEL_MEMINFO;
extern multiboot_info_t MULTIBOOT_INFO;

static addr_space_entry_t ADDR_SPACE[ADDR_SPACE_ENTRIES];

static void add_allocation(vmm_context_t *vmm_context, uint32_t virtual_addr,
                           uint32_t physical_addr, uint32_t page_count, uint32_t flags);
static void remove_allocation(vmm_context_t *vmm_context,
                              vmm_allocation_tree_t *allocation);
static void add_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                            uint32_t page_count);
static uint32_t remove_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr,
                                   uint32_t page_count, bool is_userspace);
static vmm_context_t create_empty_ctx();

vmm_context_t
vmm_init() {
    // Kernel text
    ADDR_SPACE[0] = (addr_space_entry_t) {
        .virtual_start = KERNEL_MEMINFO.text_virtual_start,
        .physical_start = KERNEL_MEMINFO.text_physical_start,
        .page_count = paging_page_count(KERNEL_MEMINFO.text_virtual_end -
                                        KERNEL_MEMINFO.text_virtual_start),
        .flags = PAGE_FLAG_PRESENT
    };

    // Kernel rodata
    ADDR_SPACE[1] = (addr_space_entry_t) {
        .virtual_start = KERNEL_MEMINFO.rodata_virtual_start,
        .physical_start = KERNEL_MEMINFO.rodata_physical_start,
        .page_count = paging_page_count(KERNEL_MEMINFO.rodata_virtual_end -
                                        KERNEL_MEMINFO.rodata_virtual_start),
        .flags = PAGE_FLAG_PRESENT
    };

    // Kernel data
    ADDR_SPACE[2] = (addr_space_entry_t) {
        .virtual_start = KERNEL_MEMINFO.data_virtual_start,
        .physical_start = KERNEL_MEMINFO.data_physical_start,
        .page_count = paging_page_count(KERNEL_MEMINFO.data_virtual_end -
                                        KERNEL_MEMINFO.data_virtual_start),
        .flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    };

    // Kernel bss
    //
    // TODO: make the `.bss` pages copy-on-write
    ADDR_SPACE[3] = (addr_space_entry_t) {
        .virtual_start = KERNEL_MEMINFO.bss_virtual_start,
        .physical_start = KERNEL_MEMINFO.bss_physical_start,
        .page_count = paging_page_count(KERNEL_MEMINFO.bss_virtual_end - KERNEL_MEMINFO.bss_virtual_start),
        .flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    };

    // Kernel heap
    ADDR_SPACE[4] = (addr_space_entry_t) {
        .virtual_start = KERNEL_HEAP_VIRT_START,
        .physical_start = KERNEL_HEAP_PHYS_START,
        .page_count = paging_page_count(KERNEL_HEAP_SIZE),
        .flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    };

    // Framebuffer
    struct multiboot_tag_framebuffer_common *framebuffer_info =
        multiboot_framebuffer_info(MULTIBOOT_INFO.addr);
    ADDR_SPACE[5] = (addr_space_entry_t) {
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
    uint64_t user_page_count = KERNEL_MEMINFO.higher_half_base / PAGE_SIZE;
    uint64_t kernel_page_count = total_page_count - user_page_count;
    // Separate the userspace addresses from the kernel ones:
    add_free_blocks(&vmm_context, 0, user_page_count);
    add_free_blocks(&vmm_context, KERNEL_MEMINFO.higher_half_base, kernel_page_count);

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
    bool is_userspace = flags & PAGE_FLAG_USER;

    // TODO handle flags
    ASSERT(paging_is_aligned(virtual_addr), "cannot map unaligned address: %#x", virtual_addr);
    ASSERT(paging_is_aligned(physical_addr), "cannot map to unaligned address: %#x", physical_addr);

    if (!vmm_context->free_blocks) {
        // XXX handle this more gracefully
        PANIC("Out of memory");
    }

    uint32_t addr = remove_free_blocks(vmm_context, virtual_addr, page_count, is_userspace);
    add_allocation(vmm_context, addr, physical_addr, page_count, flags);

    return (void *)addr;
}

void
vmm_unmap_pages(vmm_context_t *vmm_context, uint32_t virtual_addr, uint32_t page_count) {
    ASSERT(paging_is_aligned(virtual_addr), "cannot unmap unaligned address: %#x", virtual_addr);

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

paging_context_t
vmm_clone_paging_context(vmm_context_t *vmm_ctx, paging_context_t paging_ctx) {
    uint32_t physical_addr = (uint32_t)pmm_alloc_page();
    page_table_t *page_directory = (page_table_t *)vmm_map_pages(vmm_ctx, 0, physical_addr, 1,
                                   PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
    memcpy(page_directory, paging_ctx.page_directory, sizeof(page_table_t));

    page_directory->entries[PAGE_TABLE_SIZE - 1] = physical_addr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

    return (paging_context_t) {
        .page_directory = page_directory,
        .page_tables = paging_ctx.page_tables,
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

static bool
is_addr_in_user_range(uint32_t virtual_addr, uint32_t page_count) {
    uint64_t range_end = virtual_addr + (uint64_t)page_count * PAGE_SIZE;

    return range_end < KERNEL_MEMINFO.higher_half_base;
}

static bool
is_addr_ok_for_mode(uint32_t virtual_addr, uint32_t page_count, bool is_userspace) {
    if (is_userspace) {
        return is_addr_in_user_range(virtual_addr, page_count);
    } else {
        return virtual_addr >= KERNEL_MEMINFO.virtual_start;
    }
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
                   uint32_t page_count, bool is_userspace) {
    ASSERT(vmm_context->free_blocks, "out of memory (no free blocks)");

    vmm_free_blocks_t *free_blocks = vmm_context->free_blocks;
    vmm_free_blocks_t *prev = NULL;

    while (free_blocks) {
        // If the caller didn't request a specific address, check if the current
        // address is acceptable given the current access ring.
        bool select_current_addr = !virtual_addr
                                   && is_addr_ok_for_mode(free_blocks->virtual_addr, page_count, is_userspace);

        // Alternatively, if the caller requested a _specific_ virtual address, try to find it.
        if (select_current_addr || is_addr_in_range(virtual_addr, free_blocks)) {
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
        uint32_t page_offset = virtual_addr ? (virtual_addr - free_blocks->virtual_addr) / PAGE_SIZE : 0;

        // Are there enough free pages to satisfy the request?
        if (page_count > free_blocks->page_count - page_offset) {
            PANIC("out of memory");
        }

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

    return virtual_addr;
}

static void
merge_or_insert_free_blocks(vmm_free_blocks_t *free_blocks, uint32_t virtual_addr,
                            uint32_t page_count) {
    bool blocks_merged = false;
    if (free_blocks) {
        bool is_userspace_block = is_addr_in_user_range(free_blocks->virtual_addr, free_blocks->page_count);
        bool is_userspace_addr = is_addr_in_user_range(virtual_addr, page_count);

        // Don't merge the userspace ranges with the kernel ones.
        if (!(is_userspace_block ^ is_userspace_addr)) {
            goto done;
        }

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

done:
    if (!blocks_merged) {
        vmm_free_blocks_t *new_block = (vmm_free_blocks_t *)kmalloc(sizeof(vmm_free_blocks_t));
        vmm_free_blocks_t *next = free_blocks ? free_blocks->next : NULL;
        *new_block = (vmm_free_blocks_t) {
            .virtual_addr = virtual_addr,
            .page_count = page_count,
            .next = next,
        };

        if (free_blocks) {
            free_blocks->next = new_block;
        } else {
            free_blocks = new_block;
        }
    }
}

static void
add_free_blocks(vmm_context_t *vmm_context, uint32_t virtual_addr, uint32_t page_count) {
    vmm_free_blocks_t *free_blocks = vmm_context->free_blocks;
    vmm_free_blocks_t *prev = NULL;

    while (free_blocks && virtual_addr > free_blocks->virtual_addr) {
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
