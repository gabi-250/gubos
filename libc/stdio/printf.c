#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int
printf(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    size_t len = strlen(fmt);
    // XXX implement me
    return len;
}
