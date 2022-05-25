#ifndef __PAGE_FAULT_H__
#define __PAGE_FAULT_H__

#include <stdint.h>
#include "idt.h"
#include "interrupts/handlers.h"

// ======================================================================
// Page fault exception error code flags
//
// See also: Figure 6-11 from Chapter 6 (Interrupt and exception handling) of
// Intel® 64 and IA-32 Architectures Software Developer's Manual, Volume 3.
// ======================================================================

// If set to 0, the page fault was caused by a non-present page (P flag of one of the paging
// structures used to translate the address was set to 0).
//
// If set to 1, the page fault was caused by a page protection violation.
#define PAGING_ERR_CODE_P     1
// The access causing the page-fault exception was a write (if set to 0, the
// access was a read).
#define PAGING_ERR_CODE_WR   (1 << 1)
// The exception was caused by a user-mode access (if set to 0, it was caused by
// a supervisor-mode access).
#define PAGING_ERR_CODE_US   (1 << 2)
// Indicates no there is no translation for the address because a reserved bit
// was set in one of the paging-structures of the address.
#define PAGING_ERR_CODE_RSVD (1 << 3)
// The exception was caused by an instruction fetch.
#define PAGING_ERR_CODE_ID   (1 << 4)
// The exception was caused by an access not allowed by the protection keys.
#define PAGING_ERR_CODE_PK   (1 << 5)
// The exception was caused by a shadow-stack access (otherwise set to 0).
#define PAGING_ERR_CODE_SS   (1 << 6)
// Violation of SGX access control.
#define PAGING_ERR_CODE_SGX  (1 << 15)

void page_fault_handler(interrupt_state_t *, uint32_t);

#endif /* __PAGE_FAULT_H__ */
