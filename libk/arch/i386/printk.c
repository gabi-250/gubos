#include "printk.h"
#include "tty.h"
#include "string.h"

#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>

#define INT_ARG_MAX_LEN 11
#define LONG_LONG_ARG_MAX_LEN 20

static int
putchar(int ic) {
    char c = (char) ic;
    tty_puts(&c, sizeof(c));
    return ic;
}

static bool
print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i = 0; i < length; i++) {
        if (putchar(bytes[i]) == EOF) {
            return false;
        }
    }
    return true;
}

char *
itoa(long long value, char *str, int base) {
    // XXX assume base is 10
    if (base != 10) {
        return NULL;
    }
    size_t i = 0;
    bool negative = value < 0;
    if (!value) {
        str[i++] = '0';
    } else {
        if (negative) {
            str[i++] = '-';
            value = -value;
        }
        while (value) {
            str[i++] = (char)(value % 10 + '0');
            value /= 10;
        }
    }
    str[i] = '\0';
    if (negative) {
        strrev(str + 1);
    } else {
        strrev(str);
    }
    return str;
}

int
printk(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);

    size_t i = 0;
    size_t len = strlen(fmt);

    while (i < len) {
        size_t start = i;
        while (i < len && fmt[i] != '%') {
            ++i;
        }
        if (i > start) {
            print(fmt + start, i - start);
        }

        if (fmt[i] == '%') {
            if (i == len) {
                // XXX error
                return len;
            }
            switch (fmt[++i]) {
                case '%':
                    print(fmt + i, 1);
                    break;
                case 'd':
                case 'i':
                    int int_arg = va_arg(params, int);
                    char printable_int_arg[INT_ARG_MAX_LEN];
                    itoa((long long)int_arg, printable_int_arg, 10);
                    print(printable_int_arg, strlen(printable_int_arg));
                    break;
                case 'l':
                    if (i < len - 2 && fmt[i + 1] == 'l' && fmt[i + 2] == 'd') {
                        long long long_arg = va_arg(params, long long);
                        char printable_long_arg[LONG_LONG_ARG_MAX_LEN];
                        itoa((long long)long_arg, printable_long_arg, 10);
                        print(printable_long_arg, strlen(printable_long_arg));
                        i += 2;
                    } else {
                        // XXX error
                    }
                    break;
                case 'c':
                    // Arguments of types narrower than int
                    // are promoted to int
                    char char_arg = va_arg(params, int);
                    print(&char_arg, 1);
                    break;
                case 's':
                    char *str_arg = va_arg(params, char *);
                    print(str_arg, strlen(str_arg));
                    break;
                default:
                    // XXX error
                    break;
            }
            ++i;
        }
    }
    return len;
}
