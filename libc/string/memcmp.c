#include <string.h>

int
memcmp(const void *ptr1, const void *ptr2, size_t num) {
    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;
    for (size_t i = 0; i < num; ++i) {
        const char v = p1[i] - p2[i];
        if (v != 0) {
            return v;
        }
    }
    return 0;
}
