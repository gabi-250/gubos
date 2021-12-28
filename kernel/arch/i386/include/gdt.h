#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>
#include <stdbool.h>

// These must be kept in sync with the gdt_init logic
#define GDT_KERNEL_CODE_SEGMENT 0x8
#define GDT_KERNEL_DATA_SEGMENT 0x10
#define GDT_USER_CODE_SEGMENT 0x18
#define GDT_USER_DATA_SEGMENT 0x20

#endif /* __GDT_H__ */
