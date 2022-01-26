#ifndef __PANIC_H__
#define __PANIC_H__

#define PANIC_MESSAGE \
    "======================================\n"\
    " /\\/\\/\\/\\/\\/\\/\\ panic /\\/\\/\\/\\/\\/\\/\\\n"\
    "======================================\n"

#define PANIC(fmt, ...) \
    panic(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

void __attribute__ ((noreturn)) panic(const char *, int, const char *, ...);

#endif /* __PANIC_H__ */
