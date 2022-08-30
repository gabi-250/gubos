#include <string.h>

void *
memset(void *ptr, int value, size_t num) {
    unsigned char *buf = (unsigned char *) ptr;

    for (size_t i = 0; i < num; ++i) {
        buf[i] = (unsigned char)value;
    }

    return ptr;
}
