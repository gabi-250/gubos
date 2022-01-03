#ifndef __IDT_H__
#define __IDT_H__

// Some constants to help with setting up the flags for each interrupt gate.
#define IDT_TASK_GATE_FLAGS                0b00000101
#define IDT_INT_GATE_FLAGS                 0b00000110
#define IDT_TRAP_GATE_FLAGS                0b00000111
#define IDT_KERNEL_DPL                     0b00000000
#define IDT_SEG_PRESENT                    0b10000000
#define IDT_32_BIT_GATE_SIZE               0b00001000
#define IDT_RESERVED_INT_COUNT             32

// The interrupts defined in the Intel manual.
#define IDT_DIVIDE_ERROR_EXCEPTION         0
#define IDT_DEBUG_EXCEPTION                1
#define IDT_NMI_INTERRUPT                  2
#define IDT_BREAKPOINT_EXCEPTION           3
#define IDT_OVERFLOW_EXCEPTION             4
#define IDT_BOUND_RANGE_EXCEEDED_EXCEPTION 5
#define IDT_INVALID_OPCODE_EXCEPTION       6
#define IDT_DEVICE_NOT_AVAILABLE_EXCEPTION 7
#define IDT_DOUBLE_FAULT_EXCEPTION         8
#define IDT_COPROCESSOR_SEGMENT_OVERRUN    9
#define IDT_INVALID_TSS_EXCEPTION          10
#define IDT_SEGMENT_NOT_PRESENT            11
#define IDT_STACK_FAULT_EXCEPTION          12
#define IDT_GENERAL_PROTECTION_EXCEPTION   13
#define IDT_PAGE_FAULT_EXCEPTION           14
#define IDT_X87_FPU_FLOATING_POINT_ERROR   16
#define IDT_ALIGNMENT_CHECK_EXCEPTION      17
#define IDT_MACHINE_CHECK_EXCEPTION        18
#define IDT_SIMD_FLOATING_POINT_EXCEPTION  19
#define IDT_VIRTUALIZATION_EXCEPTION       20
#define IDT_CONTROL_PROTECTION_EXCEPTION   21

#define IDT_INT_HAS_ERROR_CODE(X) \
    (X == IDT_DOUBLE_FAULT_EXCEPTION \
     || X == IDT_INVALID_TSS_EXCEPTION \
     || X == IDT_SEGMENT_NOT_PRESENT \
     || X == IDT_STACK_FAULT_EXCEPTION \
     || X == IDT_GENERAL_PROTECTION_EXCEPTION \
     || X == IDT_PAGE_FAULT_EXCEPTION \
     || X == IDT_ALIGNMENT_CHECK_EXCEPTION \
     || X == IDT_CONTROL_PROTECTION_EXCEPTION)

#endif /* __IDT_H__ */
