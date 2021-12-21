#include <stdint.h>
#include <stdbool.h>
#include "printk.h"
#include "gdt.h"

#define IDT_TASK_GATE_FLAGS       0b00000101
#define IDT_INT_GATE_FLAGS        0b00000110
#define IDT_TRAP_GATE_FLAGS       0b00000111
#define IDT_KERNEL_DPL            0b00000000
#define IDT_SEG_PRESENT           0b10000000
#define IDT_32_BIT_GATE_SIZE      0b00001000

// The number of entries in the interrupt descriptor table
#define IDT_GATE_DESCRIPTOR_COUNT 256

// The IDT can contain 3 kinds of gate descriptors:
// * task-gate: the same as the task gates used in the GDT/LDT
// * interrupt-gate
// * trap-gate
typedef struct idt_gate_descriptor {
    uint16_t addr_low;
    uint16_t segment_selector;
    uint8_t reserved;
    // | 7 | 6  5 | 4       0 |
    // | P | DPL  | 0 D X X K |
    //
    // P   - the segment present flag
    // DPL - the descriptor privilege level
    // D   - 0 for task gates, or the size of gate (1 = 32 bits, 0 = 16 bits)
    //       for interrupt/trap gates
    // XX  - 10 for task gates, 11 for interrupt/trap gates
    // K   - trap / interrupt (trap = 0, interrupt = 1)
    uint8_t flags;
    uint16_t addr_high;
} __attribute__((packed)) idt_gate_descriptor_t;

typedef struct idt_register {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_register_t;

 __attribute__ ((aligned(8)))
static idt_gate_descriptor_t idt_descriptors[IDT_GATE_DESCRIPTOR_COUNT];
static idt_register_t idtr;

__attribute__ ((interrupt)) void
default_exception_handler(void *) {
    printk("handling interrupt...\n");
}

void
set_idt_gate(uint8_t vector, void *isr, uint8_t flags) {
    idt_gate_descriptor_t *gate = &idt_descriptors[vector];

    gate->addr_low = (uint32_t)isr & 0xffff;
    gate->segment_selector = GDT_KERNEL_CODE_SEGMENT;
    gate->flags = flags;
    gate->addr_high = ((uint32_t)isr >> 16) & 0xffff;
    gate->reserved = 0;
}

void
init_idt() {
    idtr.base = (uint32_t)&idt_descriptors;
    idtr.limit = sizeof(idt_gate_descriptor_t) * IDT_GATE_DESCRIPTOR_COUNT - 1;

    uint8_t flags = IDT_TRAP_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL | IDT_32_BIT_GATE_SIZE;
    for (uint16_t i = 0; i < 32; ++i) {
        set_idt_gate(i, default_exception_handler, flags);
    }

    flags = IDT_INT_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL | IDT_32_BIT_GATE_SIZE;
    for (uint16_t i = 32; i < IDT_GATE_DESCRIPTOR_COUNT; ++i) {
        set_idt_gate(i, default_exception_handler, flags);
    }
    // load the IDT register and enable interrupts
    asm volatile("lidt %0\n\t"
                 "sti"
                 ::"memory"(idtr));
}
