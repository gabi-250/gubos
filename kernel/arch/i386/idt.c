#include <stdint.h>
#include <stdbool.h>
#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "portio.h"
#include "ps2.h"
#include "paging.h"

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
default_exception_handler(interrupt_state_t *state) {
    printk_debug("handling interrupt (eflags=%#x, cs=%d, eip=%d)\n",
            state-> eflags,
            state->cs,
            state->eip);
}

__attribute__ ((interrupt)) void
default_exception_handler_with_err(interrupt_state_t *state, unsigned int err_code) {
    printk_debug("handling interrupt (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}

__attribute__ ((interrupt)) void
page_fault_exception_handler(interrupt_state_t *state, unsigned int err_code) {
    paging_handle_fault(state, err_code);
}

__attribute__ ((interrupt)) void
timer_irq_handler(interrupt_state_t *) {
    pic_send_eoi(PIC_IRQ0);
}

__attribute__ ((interrupt)) void
keyboard_irq_handler(interrupt_state_t *) {
    ps2_handle_irq1();
}

static void
set_idt_gate(uint8_t vector, void *isr, uint8_t flags) {
    idt_gate_descriptor_t *gate = &idt_descriptors[vector];

    gate->addr_low = (uint32_t)isr & 0xffff;
    gate->segment_selector = GDT_KERNEL_CODE_SEGMENT;
    gate->flags = flags;
    gate->addr_high = ((uint32_t)isr >> 16) & 0xffff;
    gate->reserved = 0;
}

static void
init_hardware_interrupts() {
    uint8_t flags = IDT_TRAP_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL | IDT_32_BIT_GATE_SIZE;
    // Interrupts 32 - 46 are hardware interrupts.
    set_idt_gate(IDT_RESERVED_INT_COUNT, timer_irq_handler, flags);
    set_idt_gate(IDT_RESERVED_INT_COUNT + 1, keyboard_irq_handler, flags);
    pic_init();
    pic_clear_mask(PIC_IRQ0);
    pic_clear_mask(PIC_IRQ1);
}

void
idt_init() {
    idtr.base = (uint32_t)&idt_descriptors;
    idtr.limit = sizeof(idt_gate_descriptor_t) * IDT_GATE_DESCRIPTOR_COUNT - 1;
    // The entries in the 0-31 range are architecture-defined
    // interrupts/exceptions
    uint8_t flags = IDT_TRAP_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL | IDT_32_BIT_GATE_SIZE;
    for (uint16_t i = 0; i < IDT_PAGE_FAULT_EXCEPTION; ++i) {
        if (IDT_INT_HAS_ERROR_CODE(i)) {
            set_idt_gate(i, default_exception_handler_with_err, flags);
        } else {
            set_idt_gate(i, default_exception_handler, flags);
        }
    }
    for (uint16_t i = IDT_X87_FPU_FLOATING_POINT_ERROR; i < IDT_RESERVED_INT_COUNT; ++i) {
        if (IDT_INT_HAS_ERROR_CODE(i)) {
            set_idt_gate(i, default_exception_handler_with_err, flags);
        } else {
            set_idt_gate(i, default_exception_handler, flags);
        }
    }
    set_idt_gate(IDT_PAGE_FAULT_EXCEPTION, page_fault_exception_handler, flags);
    init_hardware_interrupts();
    flags = IDT_INT_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL | IDT_32_BIT_GATE_SIZE;
    // The rest are user-defined interrupts
    for (uint16_t i = IDT_RESERVED_INT_COUNT + 2; i < IDT_GATE_DESCRIPTOR_COUNT; ++i) {
        set_idt_gate(i, default_exception_handler, flags);
    }
    // Load the IDT register and enable interrupts
    asm volatile("lidt %0\n\t"
                 "sti"
                 ::"memory"(idtr));
}
