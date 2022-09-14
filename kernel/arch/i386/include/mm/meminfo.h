#ifndef __KERNEL_MEMINFO_H__
#define __KERNEL_MEMINFO_H__

#include <stdint.h>
#include <stddef.h>

typedef struct kernel_meminfo {
    uint32_t physical_start;
    uint32_t physical_end;
    uint32_t virtual_start;
    uint32_t virtual_end;
    uint32_t text_virtual_start;
    uint32_t text_physical_start;
    uint32_t text_virtual_end;
    uint32_t rodata_virtual_start;
    uint32_t rodata_physical_start;
    uint32_t rodata_virtual_end;
    uint32_t data_virtual_start;
    uint32_t data_physical_start;
    uint32_t data_virtual_end;
    uint32_t bss_virtual_start;
    uint32_t bss_physical_start;
    uint32_t bss_virtual_end;
    uint32_t higher_half_base;
    uint32_t stack_top;
} kernel_meminfo_t;

size_t kernel_meminfo_size(void);

#endif /* __KERNEL_MEMINFO_H__ */
