#include "printk.h"
#include "itoa.h"
#include "tty.h"
#include "vga.h"
#include "string.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

// XXX
#define ARG_MAX_LEN 256
#define PRINTK_LOG_COLOR_DEBUG vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_LIGHT_BLUE)
#define PRINTK_LOG_COLOR_INFO vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_BLUE)
#define PRINTK_LOG_COLOR_WARN vga_entry_color(VGA_COLOR_BROWN, VGA_COLOR_LIGHT_BLUE)
#define PRINTK_LOG_COLOR_ERROR vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_BLUE)

static int
putchar(uint8_t color, int ic) {
    char c = (char) ic;
    tty_puts(&c, sizeof(c), color);
    return ic;
}

static uint8_t
log_level_to_vga_color(uint8_t log_level) {
    switch (log_level) {
        case PRINTK_DEBUG:
            return PRINTK_LOG_COLOR_DEBUG;
        case PRINTK_INFO:
            return PRINTK_LOG_COLOR_INFO;
        case PRINTK_WARN:
            return PRINTK_LOG_COLOR_WARN;
        case PRINTK_ERROR:
            return PRINTK_LOG_COLOR_ERROR;
        default:
            return PRINTK_LOG_COLOR_INFO;
    }
}

static bool
print(uint8_t color, const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i = 0; i < length; i++) {
        if (putchar(color, bytes[i]) == EOF) {
            return false;
        }
    }
    return true;
}

int
vprintk(uint8_t log_level, const char *fmt, va_list params) {
    char formatted_arg[ARG_MAX_LEN];
    size_t i = 0;
    size_t len = strlen(fmt);
    uint8_t color = log_level_to_vga_color(log_level);

    while (i < len) {
        size_t start = i;
        while (i < len && fmt[i] != '%') {
            ++i;
        }
        if (i > start) {
            print(color, fmt + start, i - start);
        }
        if (fmt[i] == '%') {
            bool flags = PRINTK_FLAGS_DEFAULT;
            if (i == len) {
                goto missing_fmt_specifier;
            }
            // Peek at the flags
            switch (fmt[i + 1]) {
                case '#':
                    flags = flags | PRINTK_FLAGS_HASH;
                    ++i;
                    break;
                case '-':
                case '+':
                case ' ':
                case '0':
                    // XXX maybe handle other flags
                    ++i;
                default:
                    break;
            }
            if (i == len ) {
                goto missing_fmt_specifier;
            }
            switch (fmt[++i]) {
                case '%':
                    print(color, fmt + i, 1);
                    break;
                case 'd':
                case 'i':
                    itoa(va_arg(params, int), formatted_arg);
                    print(color, formatted_arg, strlen(formatted_arg));
                    break;
                case 'x':
                case 'X':
                    if (flags & PRINTK_FLAGS_HASH) {
                        print(color, "0x", 2);
                    }
                    uitoa_hex(va_arg(params, unsigned int), formatted_arg);
                    if (fmt[i] == 'X') {
                        strupper(formatted_arg);
                    }
                    print(color, formatted_arg, strlen(formatted_arg));
                    break;
                case 'l':
                    if (i < len - 2 && fmt[i + 1] == 'l') {
                        // ll-prefix
                        switch (fmt[i + 2]) {
                            case 'd':
                            case 'i':
                                itoa(va_arg(params, long long), formatted_arg);
                                break;
                            case 'u':
                                uitoa(va_arg(params, unsigned long long), formatted_arg);
                                break;
                            case 'x':
                            case 'X':
                                if (flags & PRINTK_FLAGS_HASH) {
                                    print(color, "0x", 2);
                                }
                                uitoa_hex(va_arg(params, unsigned long long), formatted_arg);
                                if (fmt[i] == 'X') {
                                    strupper(formatted_arg);
                                }
                                break;
                            default:
                                goto unknown_fmt_specifier;
                        }
                        print(color, formatted_arg, strlen(formatted_arg));
                        i += 2;
                    } else {
                        goto unknown_fmt_specifier;
                    }
                    break;
                case 'c':
                    // Arguments of types narrower than int
                    // are promoted to int
                    putchar(color, va_arg(params, int));
                    break;
                case 's':
                    char *str_arg = va_arg(params, char *);
                    print(color, str_arg, strlen(str_arg));
                    break;
                default:
                    goto unknown_fmt_specifier;
            }
            ++i;
        }
    }
    return len;
missing_fmt_specifier:
unknown_fmt_specifier:
    // XXX set errno
    return -1;
}

int
printk(uint8_t log_level, const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    int ret = vprintk(log_level, fmt, params);
    va_end(params);
    return ret;
}

int
printk_debug(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    int ret = vprintk(PRINTK_DEBUG, fmt, params);
    va_end(params);
    return ret;
}

int
printk_info(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    int ret = vprintk(PRINTK_INFO, fmt, params);
    va_end(params);
    return ret;
}

int
printk_warn(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    int ret = vprintk(PRINTK_WARN, fmt, params);
    va_end(params);
    return ret;
}

int
printk_err(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    int ret = vprintk(PRINTK_ERROR, fmt, params);
    va_end(params);
    return ret;
}
