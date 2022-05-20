#ifndef __VMM_H__
#define __VMM_H__

#include <stdint.h>
#include "kernel_meminfo.h"

// ======================================================================
// Paging constants
// ======================================================================
#define PAGE_DIRECTORY_START 22
#define PAGE_TABLE_START     12
// 4 MB pages
#define KERNEL_PAGE_SIZE     (1 << 22)
#define PAGE_TABLE_SIZE      1024

// ======================================================================
// Page table entry flags
// ======================================================================
// Present flag.
#define PAGE_FLAG_PRESENT         1
// Read/write flag. If 0, writes aren't allowed.
#define PAGE_FLAG_WRITE          (1 << 1)
// User/supervisor flag. If 0, user-mode accesses aren't allowed.
#define PAGE_FLAG_USER           (1 << 2)
// Page-level write-through flag. It (indirectly) determines the memory type
// used to access the page referenced by this entry.
#define PAGE_FLAG_WRITE_THROUGH  (1 << 3)
// The cache disable flag. It (indirectly) determines the memory type used to
// access this entry.
#define PAGE_FLAG_CACHE_DISABLE  (1 << 4)
// The accessed flag. If 1, the page referenced by this entry has been accessed.
#define PAGE_FLAG_ACCESSED       (1 << 5)
// The dirty flag. If 1, the page referenced by this entry has been written to.
#define PAGE_FLAG_DIRTY          (1 << 6)
// The page size flag. Must be 1 if 4M pages are in use.
#define PAGE_FLAG_PAGE_SIZE      (1 << 7)
// The global flag.Iif CR4.PGE = 1, determines whether the translation is
// global.
#define PAGE_FLAG_GLOBAL         (1 << 8)

#define PAGE_DIRECTORY_INDEX(addr) ((addr) >> PAGE_DIRECTORY_START)
#define PAGE_TABLE_INDEX(addr) (((addr) >> PAGE_TABLE_START) & ((1 << PAGE_TABLE_START) - 1))

// Create a page directory entry (4MB pages) from the specified physical address
//
// The format of a page directory entry (with 4MB pages) is:
// | 31                                    22| 21        17 | 16                 13    | 12  | 11    0 |
// |---------------------------------------------------------------------------------------------------|
// | bits 31:22 of address of 4MB page frame | 0 (reserved) | bits 39:32 of address[2] | PAT | flags   |
//
// [2] This example illustrates a processor in which MAXPHYADDR is 36. If this
// value is larger or smaller, the number of bits reserved in positions 20:13 of
// a PDE mapping a 4-MByte page will change.
#define PHYSICAL_ADDR_TO_PDE(addr) ((addr >> 22) << 22)

typedef struct page_directory {
    uint32_t entries[PAGE_TABLE_SIZE];
} __attribute__ ((aligned(4096))) page_directory_t;

typedef struct page_table {
    uint32_t entries[PAGE_TABLE_SIZE];
} __attribute__ ((aligned(4096))) page_table_t;


void vmm_set_page_directory(uint32_t);
void vmm_map_addr(void *, uint32_t);

#endif /* __VMM_H__ */
