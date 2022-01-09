#ifndef __PAGING_H__
#define __PAGING_H__

#include "idt.h"

// ======================================================================
// Page fault exception error code flags
// ======================================================================

// The exception is caused by the fact that the P flag of one of the paging
// structures used to translate the address was set to 0.
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

void paging_handle_fault(interrupt_state_t *, unsigned int);

#endif /* __PAGING_H__ */
