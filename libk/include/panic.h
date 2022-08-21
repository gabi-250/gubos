#ifndef __PANIC_H__
#define __PANIC_H__

#define PANIC_MESSAGE \
    "======================================\n"\
    " /\\/\\/\\/\\/\\/\\/\\ panic /\\/\\/\\/\\/\\/\\/\\\n"\
    "======================================\n"

#define PANIC(fmt, ...) \
    panic(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#define ASSERT(expr, fmt, ...) \
    if (!(expr)) panic(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

void __attribute__ ((noreturn)) panic(const char *, int, const char *, ...);
void __attribute__ ((noreturn)) halt(void);

#endif /* __PANIC_H__ */
