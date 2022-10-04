#include <stdio.h>

FILE *
fopen(const char *, const char *) {
    return NULL;
}

FILE *
fdopen(int, const char *) {
    return NULL;
}

FILE *
freopen(const char *, const char *, FILE *) {
    return NULL;
}

int
feof(FILE *) {
    return 0;
}

long
ftell(FILE *) {
    return 0;
}

size_t
fread(void *, size_t, size_t, FILE *) {
    return 0;
}

size_t
fwrite(const void *, size_t, size_t, FILE *) {
    return 0;
}

int
fseek(FILE *, long, int) {
    return 0;
}

int
fclose(FILE *) {
    return 0;
}

int
fflush (FILE *) {
    return 0;
}

int
fprintf (FILE *, const char *, ...) {
    return 0;
}


int
vdprintf(int, const char *, va_list) {
    return 0;
}

int
vfprintf(FILE *, const char *, va_list) {
    return 0;
}

int
vprintf(const char *, va_list) {
    return 0;
}

int
vsnprintf(char *, size_t, const char *, va_list) {
    return 0;
}

int
vsprintf(char *, const char *, va_list) {
    return 0;
}


int
sprintf(char *, const char *, ...) {
    return 0;
}

