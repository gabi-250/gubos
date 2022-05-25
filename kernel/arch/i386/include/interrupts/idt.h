#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

// Some constants to help with setting up the flags for each interrupt gate.
#define IDT_TASK_GATE_FLAGS                0b00000101
#define IDT_INT_GATE_FLAGS                 0b00000110
#define IDT_TRAP_GATE_FLAGS                0b00000111
#define IDT_KERNEL_DPL                     0b00000000
#define IDT_SEG_PRESENT                    0b10000000
#define IDT_32_BIT_GATE_SIZE               0b00001000
#define IDT_RESERVED_INT_COUNT             32

#endif /* __IDT_H__ */
