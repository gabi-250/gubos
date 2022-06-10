#include <panic.h>
#include <printk.h>

void
__attribute__ ((noreturn))
panic(const char *file, int line, const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    printk_err("%s", PANIC_MESSAGE);
    printk_err("%s:%d: ", file, line);
    vprintk(PRINTK_ERROR, fmt, params);
    va_end(params);
    // XXX: add more debugging info.
    asm volatile("cli");
    for (;;) {
        asm volatile("hlt");
    }
}
