#include <stdint.h>
#include "interrupts/handlers.h"
#include "interrupts/page_fault.h"
#include "panic.h"

__attribute__((interrupt)) void
divide_error_exception_handler(interrupt_state_t * state) {
    PANIC("divide_error_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
debug_exception_handler(interrupt_state_t * state) {
    PANIC("debug_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
nmi_interrupt_handler(interrupt_state_t * state) {
    PANIC("nmi_interrupt (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
breakpoint_exception_handler(interrupt_state_t * state) {
    PANIC("breakpoint_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
overflow_exception_handler(interrupt_state_t * state) {
    PANIC("overflow_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
bound_range_exceeded_exception_handler(interrupt_state_t * state) {
    PANIC("bound_range_exceeded_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
invalid_opcode_exception_handler(interrupt_state_t * state) {
    PANIC("invalid_opcode_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
device_not_available_exception_handler(interrupt_state_t * state) {
    PANIC("device_not_available_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
double_fault_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("double_fault_exception (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}

__attribute__((interrupt)) void
coprocessor_segment_overrun_handler(interrupt_state_t * state) {
    PANIC("coprocessor_segment_overrun (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
invalid_tss_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("invalid_tss_exception (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}
__attribute__((interrupt)) void
segment_not_present_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("segment_not_present (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}

__attribute__((interrupt)) void
stack_fault_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("stack_fault_exception (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}

__attribute__((interrupt)) void
general_protection_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("general_protection_exception (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}

__attribute__((interrupt)) void
page_fault_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    page_fault_handler(state, err_code);
}

__attribute__((interrupt)) void
x87_fpu_floating_point_error_handler(interrupt_state_t * state) {
    PANIC("x87_fpu_floating_point_error (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
alignment_check_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("alignment_check_exception (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}
__attribute__((interrupt)) void
machine_check_exception_handler(interrupt_state_t * state) {
    PANIC("machine_check_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
simd_floating_point_exception_handler(interrupt_state_t * state) {
    PANIC("simd_floating_point_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
virtualization_exception_handler(interrupt_state_t * state) {
    PANIC("virtualization_exception (eflags=%#x, cs=%d, eip=%d)\n",
            state->eflags,
            state->cs,
            state->eip);
}

__attribute__((interrupt)) void
control_protection_exception_handler(interrupt_state_t * state, uint32_t err_code) {
    PANIC("control_protection_exception (eflags=%#x, cs=%d, eip=%d, error=%d)\n",
            state->eflags,
            state->cs,
            state->eip,
            err_code);
}

