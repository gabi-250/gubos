#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <itoa.h>

char *
itoa(long long value, char *str) {
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

char *
uitoa(unsigned long long value, char *str) {
    size_t i = 0;
    if (!value) {
        str[i++] = '0';
    } else {
        while (value) {
            str[i++] = (char)(value % 10 + '0');
            value /= 10;
        }
    }
    str[i] = '\0';
    return str;
}

char *
uitoa_hex(unsigned long long value, char *str) {
    size_t i = 0;
    if (!value) {
        str[i++] = '0';
    } else {
        while (value) {
            int digit = value % 16;
            if (digit < 10) {
                str[i++] = (char)(digit + '0');
            } else {
                str[i++] = (char)(digit - 10 + 'a');
            }
            value /= 16;
        }
    }
    str[i] = '\0';
    strrev(str);
    return str;
}
