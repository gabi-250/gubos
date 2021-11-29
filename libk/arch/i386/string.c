#include "string.h"

size_t
strlen(const char *s) {
    size_t len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}

char *
strrev(char *s) {
    int i = strlen(s) - 1;
    int j = 0;
    while (i > j) {
        // XOR swap
        s[i] = s[i] ^ s[j];
        s[j] = s[i] ^ s[j];
        s[i] = s[i] ^ s[j];
        ++j;
        --i;
    }
    return s;
}
