#include <stdbool.h>
#include <string.h>

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

char *
strupper(char *s) {
    for (size_t i = 0; i < strlen(s); ++i) {
        if (islower(s[i])) {
            s[i] = toupper(s[i]);
        }
    }
    return s;
}

bool
islower(char c) {
    return c >= 'a' && c <= 'z';
}

char
toupper(char c) {
    if (islower(c)) {
        return c + ('A' - 'a');
    } else {
        return c;
    }
}
