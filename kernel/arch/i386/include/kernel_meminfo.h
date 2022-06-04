#ifndef __KERNEL_MEMINFO_H__
#define __KERNEL_MEMINFO_H__

#include <stdint.h>

typedef struct kernel_meminfo {
    uint32_t physical_start;
    uint32_t physical_end;
    uint32_t virtual_start;
    uint32_t virtual_end;
    uint32_t higher_half_base;
    uint32_t stack_top;
} kernel_meminfo_t;

#endif /* __KERNEL_MEMINFO_H__ */
