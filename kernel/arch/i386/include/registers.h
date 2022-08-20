#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stdint.h>

typedef struct registers {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t ebp;
} registers_t;

#endif //__REGISTERS_H__
