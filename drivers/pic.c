#include <stdint.h>
#include "pic.h"
#include "portio.h"

void
pic_init() {
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    // Send ICW1...
    outb(PIC1_CONTROL, PIC_ICW1_IC4 | PIC_ICW1_INIT);
    outb(PIC2_CONTROL, PIC_ICW1_IC4 | PIC_ICW1_INIT);
    // ICW2 is sent to the data registers of the PIC. Bits 3-7 of ICW2 specify
    // the base address of the interrupt vector base address to use.
    // In other words, because interrupts 0-31 are reserved, we need to remap
    // the IRQs of the PIC to different interrupt numbers. In this case IRQ 0 is
    // remapped to PIC1_INT_OFFSET and IRQ8 (handled by the secondary PIC) is
    // mapped to PIC2_INT_OFFSET.
    outb(PIC1_DATA, PIC1_INT_OFFSET);
    outb(PIC2_DATA, PIC2_INT_OFFSET);
    // ICW3 is only needed when there's more than one PIC on the system. We have
    // 2 PICs so we must send ICW3.
    // For the master PIC, we must set bit 2 of ICW3, because IRQ2 is connected
    // to the slave PIC.
    outb(PIC1_DATA, 1 << 2);
    // For the slave PIC, bits 3-7 of ICW3 need to be set to 0 (reserved), and
    // bits 0-2 specify the value of the IRQ the master PIC uses to connect to
    // it.
    outb(PIC2_DATA, 2);
    // Finally send ICW4 (we only need to set the bit that says we're in
    // 8086/8088 mode).
    outb(PIC1_DATA, PIC_ICW4_X86);
    outb(PIC2_DATA, PIC_ICW4_X86);
    // XXX is this really necessary?
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
    /*outb(PIC1_DATA, 0);*/
    /*outb(PIC2_DATA, 0);*/
}

void
pic_send_eoi(uint8_t irq) {
    if(irq >= 8) {
        outb(PIC2_CONTROL, PIC_OCW2_EOI);
    }
    outb(PIC1_CONTROL, PIC_OCW2_EOI);
}

void
pic_clear_mask(uint8_t irq) {
    uint16_t port = 0;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}
