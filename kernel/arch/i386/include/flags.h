#ifndef __FLAGS_H__
#define __FLAGS_H__

#include <stdint.h>

// Can be used to check if the interrupt enable flag is set in the FLAGS
// register
#define FLAGS_INT_ENABLE_FLAG (1 << 9)

uint32_t cpu_flags();
bool interrupts_enabled();

#endif /* __FLAGS_H__ */
