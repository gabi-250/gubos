#ifndef __FLAGS_H__
#define __FLAGS_H__

// Can be used to check if the interrupt enable flag is set in the FLAGS
// register
#define FLAGS_INT_ENABLE_FLAG (1 << 9)

#ifndef __ASSEMBLY__
#include <stdint.h>

uint32_t cpu_flags();
bool interrupts_enabled();
#endif

#endif /* __FLAGS_H__ */
