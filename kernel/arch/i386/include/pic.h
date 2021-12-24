#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

#define PIC1_CONTROL    0x20
#define PIC1_DATA       0x21
#define PIC2_CONTROL    0xa0
#define PIC2_DATA       0xa1
#define PIC1_INT_OFFSET 0x20
#define PIC2_INT_OFFSET (PIC1_INT_OFFSET + 8)
// Whether to expect Initialization Control Word 4
#define PIC_ICW1_IC4   1
// The intialization bit
#define PIC_ICW1_INIT (1 << 4)
// Whether we're in x86 mode
#define PIC_ICW4_X86  1
#define PIC_OCW2_EOI  (1 << 5)

#define PIC_IRQ0      0
#define PIC_IRQ1      1

void init_pic();
void send_eoi(uint8_t);
void pic_clear_mask(uint8_t);

#endif /* __PIC_H__ */
