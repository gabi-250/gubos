#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include <stdint.h>

// The interrupts defined in the Intel manual.
// See Table 6-1. Protected-Mode Exceptions and Interrupts
#define INT_DIVIDE_ERROR_EXCEPTION         0
#define INT_DEBUG_EXCEPTION                1
#define INT_NMI_INTERRUPT                  2
#define INT_BREAKPOINT_EXCEPTION           3
#define INT_OVERFLOW_EXCEPTION             4
#define INT_BOUND_RANGE_EXCEEDED_EXCEPTION 5
#define INT_INVALID_OPCODE_EXCEPTION       6
#define INT_DEVICE_NOT_AVAILABLE_EXCEPTION 7
#define INT_DOUBLE_FAULT_EXCEPTION         8
#define INT_COPROCESSOR_SEGMENT_OVERRUN    9
#define INT_INVALID_TSS_EXCEPTION          10
#define INT_SEGMENT_NOT_PRESENT            11
#define INT_STACK_FAULT_EXCEPTION          12
#define INT_GENERAL_PROTECTION_EXCEPTION   13
#define INT_PAGE_FAULT_EXCEPTION           14
#define INT_X87_FPU_FLOATING_POINT_ERROR   16
#define INT_ALIGNMENT_CHECK_EXCEPTION      17
#define INT_MACHINE_CHECK_EXCEPTION        18
#define INT_SIMD_FLOATING_POINT_EXCEPTION  19
#define INT_VIRTUALIZATION_EXCEPTION       20
#define INT_CONTROL_PROTECTION_EXCEPTION   21

typedef struct interrupt_state {
    uint32_t eflags;
    uint32_t cs;
    uint32_t eip;
} __attribute__((packed)) interrupt_state_t;

void divide_error_exception_handler(interrupt_state_t *);
void debug_exception_handler(interrupt_state_t *);
void nmi_interrupt_handler(interrupt_state_t *);
void breakpoint_exception_handler(interrupt_state_t *);
void overflow_exception_handler(interrupt_state_t *);
void bound_range_exceeded_exception_handler(interrupt_state_t *);
void invalid_opcode_exception_handler(interrupt_state_t *);
void device_not_available_exception_handler(interrupt_state_t *);
void double_fault_exception_handler(interrupt_state_t *, uint32_t);
void coprocessor_segment_overrun_handler(interrupt_state_t *);
void invalid_tss_exception_handler(interrupt_state_t *, uint32_t);
void segment_not_present_handler(interrupt_state_t *, uint32_t);
void stack_fault_exception_handler(interrupt_state_t *, uint32_t);
void general_protection_exception_handler(interrupt_state_t *, uint32_t);
void page_fault_exception_handler(interrupt_state_t *, uint32_t);
void x87_fpu_floating_point_error_handler(interrupt_state_t *);
void alignment_check_exception_handler(interrupt_state_t *, uint32_t);
void machine_check_exception_handler(interrupt_state_t *);
void simd_floating_point_exception_handler(interrupt_state_t *);
void virtualization_exception_handler(interrupt_state_t *);
void control_protection_exception_handler(interrupt_state_t *, uint32_t);

#endif /* __HANDLERS_H__ */
