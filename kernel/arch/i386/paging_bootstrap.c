#include <stddef.h>
#include "mm/vmm.h"
#include "kernel_meminfo.h"
#include "printk.h"
#include "panic.h"

#define KERNEL_PAGE_SIZE KERNEL_PAGE_SIZE_4MB

// The page directory of the kernel.
//
// NOTE: The kernel uses 4MB pages, so we only really need to use 256 out of the
// 1024 available entries (256 * 4MB = 1GB)
static page_table_t __attribute__((section(".data.paging_bootstrap"))) KERNEL_PAGE_DIRECTORY;

uint32_t __attribute__((section(".text.paging_bootstrap")))
paging_bootstrap(kernel_meminfo_t meminfo) {
    for (size_t i = 0; i < PAGE_TABLE_SIZE; ++i) {
        KERNEL_PAGE_DIRECTORY.entries[i] = 0;
    }

    for (uint64_t i = 0; meminfo.virtual_start + i * KERNEL_PAGE_SIZE < meminfo.virtual_end; ++i) {
        uint32_t virtual_address = meminfo.virtual_start + i * KERNEL_PAGE_SIZE;
        uint32_t physical_address = meminfo.physical_start + i * KERNEL_PAGE_SIZE;
        uint32_t entry = PAGE_DIRECTORY_INDEX(physical_address) << 22;

        KERNEL_PAGE_DIRECTORY.entries[PAGE_DIRECTORY_INDEX(virtual_address)] =
            entry | PAGE_FLAG_PRESENT | PAGE_FLAG_PAGE_SIZE | PAGE_FLAG_WRITE;
    }

    uint32_t kernel_page_directory = (uint32_t)&KERNEL_PAGE_DIRECTORY;
    return kernel_page_directory;
}
