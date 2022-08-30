#include <string.h>

void *
memcpy(void *destination, const void *source, size_t num) {
    unsigned char *to = (unsigned char *)destination;
    const unsigned char *from = (const unsigned char *)source;

    for (size_t i = 0; i < num; ++i) {
        to[i] = from[i];
    }

    return to;
}
