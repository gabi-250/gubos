#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>
#include <stdint.h>

#define EOF (-1)
#define PRINTK_FLAGS_DEFAULT 0
#define PRINTK_FLAGS_HASH    1

enum printk_log_level {
    PRINTK_DEBUG,
    PRINTK_INFO,
    PRINTK_WARN,
    PRINTK_ERROR
};

// Print a kernel log message with the specified log level.
int printk(uint8_t log_level, const char*, ...);
int vprintk(uint8_t log_level, const char *, va_list);
// Print a kernel debug message.
int printk_debug(const char*, ...);
// Print a kernel info message.
int printk_info(const char*, ...);
// Print a kernel warning message.
int printk_warn(const char*, ...);
// Print a kernel error message.
int printk_err(const char*, ...);

#endif /* __PRINTK_H__ */
