#include <stdint.h>
#include <stdbool.h>
#include <panic.h>
#include <gdt.h>
#include <interrupts/idt.h>
#include <interrupts/handlers.h>
#include <pic.h>
#include <portio.h>
#include <ps2.h>
#include <sched.h>
#include <syscall/syscall.h>
#include <panic.h>

// The number of entries in the interrupt descriptor table
#define IDT_GATE_DESCRIPTOR_COUNT 256

extern int SCHED_INIT;
extern void syscall_interrupt_handler(interrupt_state_t *state);

// The IDT can contain 3 kinds of gate descriptors:
// * task-gate: the same as the task gates used in the GDT/LDT
// * interrupt-gate (disables the IF flag, i.e. interrupts are disabled)
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
    // K   - trap / interrupt (trap = 1, interrupt = 0)
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

static
__attribute__ ((interrupt)) void
default_exception_handler(interrupt_state_t *state) {
    PANIC("received interrupt (eflags=%#x, cs=%d, eip=%d)\n",
          state->eflags,
          state->cs,
          state->eip);
}

static
__attribute__ ((interrupt)) void
timer_irq_handler(interrupt_state_t *) {
    pic_send_eoi(PIC_IRQ0);
    if (SCHED_INIT) {
        sched_context_switch();
    }
}

static
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
    uint8_t flags = IDT_TRAP_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL |
                    IDT_32_BIT_GATE_SIZE;
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
    uint8_t flags = IDT_TRAP_GATE_FLAGS | IDT_SEG_PRESENT | IDT_KERNEL_DPL |
                    IDT_32_BIT_GATE_SIZE;

    set_idt_gate(INT_DIVIDE_ERROR_EXCEPTION, divide_error_exception_handler, flags);
    set_idt_gate(INT_DEBUG_EXCEPTION, debug_exception_handler, flags);
    set_idt_gate(INT_NMI_INTERRUPT, nmi_interrupt_handler, flags);
    set_idt_gate(INT_BREAKPOINT_EXCEPTION, breakpoint_exception_handler, flags);
    set_idt_gate(INT_OVERFLOW_EXCEPTION, overflow_exception_handler, flags);
    set_idt_gate(INT_BOUND_RANGE_EXCEEDED_EXCEPTION, bound_range_exceeded_exception_handler,
                 flags);
    set_idt_gate(INT_INVALID_OPCODE_EXCEPTION, invalid_opcode_exception_handler, flags);
    set_idt_gate(INT_DEVICE_NOT_AVAILABLE_EXCEPTION, device_not_available_exception_handler,
                 flags);
    set_idt_gate(INT_DOUBLE_FAULT_EXCEPTION, double_fault_exception_handler, flags);
    set_idt_gate(INT_COPROCESSOR_SEGMENT_OVERRUN, coprocessor_segment_overrun_handler, flags);
    set_idt_gate(INT_INVALID_TSS_EXCEPTION, invalid_tss_exception_handler, flags);
    set_idt_gate(INT_SEGMENT_NOT_PRESENT, segment_not_present_handler, flags);
    set_idt_gate(INT_STACK_FAULT_EXCEPTION, stack_fault_exception_handler, flags);
    set_idt_gate(INT_GENERAL_PROTECTION_EXCEPTION, general_protection_exception_handler,
                 flags);
    set_idt_gate(INT_PAGE_FAULT_EXCEPTION, page_fault_exception_handler, flags);
    set_idt_gate(INT_X87_FPU_FLOATING_POINT_ERROR, x87_fpu_floating_point_error_handler,
                 flags);
    set_idt_gate(INT_ALIGNMENT_CHECK_EXCEPTION, alignment_check_exception_handler, flags);
    set_idt_gate(INT_MACHINE_CHECK_EXCEPTION, machine_check_exception_handler, flags);
    set_idt_gate(INT_SIMD_FLOATING_POINT_EXCEPTION, simd_floating_point_exception_handler,
                 flags);
    set_idt_gate(INT_VIRTUALIZATION_EXCEPTION, virtualization_exception_handler, flags);
    set_idt_gate(INT_CONTROL_PROTECTION_EXCEPTION, control_protection_exception_handler,
                 flags);

    init_hardware_interrupts();

    flags = IDT_INT_GATE_FLAGS | IDT_SEG_PRESENT | IDT_USER_DPL | IDT_32_BIT_GATE_SIZE;
    // The rest are user-defined interrupts
    for (uint16_t i = IDT_RESERVED_INT_COUNT + 2; i < IDT_SYSCALL_INT; ++i) {
        set_idt_gate(i, default_exception_handler, flags);
    }

    set_idt_gate(IDT_SYSCALL_INT, syscall_interrupt_handler, flags);

    for (uint16_t i = IDT_SYSCALL_INT + 1; i < IDT_GATE_DESCRIPTOR_COUNT; ++i) {
        set_idt_gate(i, default_exception_handler, flags);
    }
    // Load the IDT register and enable interrupts
    asm volatile("lidt %0\n\t"
                 "sti"
                 ::"memory"(idtr));
}
