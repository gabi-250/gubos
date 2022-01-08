#ifndef __PMM_H__
#define __PMM_H__

#include "multiboot2.h"
#include "kernel_meminfo.h"

// Initialize the physical memory manager.
void pmm_init(kernel_meminfo_t, multiboot_info_t);
// Allocate a (physical) 4M page.
void * pmm_alloc_page();
// Free the specified (physical) 4M page.
void pmm_free_page(void *);

#endif /* __PMM_H__ */
