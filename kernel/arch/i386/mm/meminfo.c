#include <mm/meminfo.h>
#include <stddef.h>

size_t
kernel_meminfo_size(void) {
    return sizeof(kernel_meminfo_t);
}
