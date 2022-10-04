#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>

int abs(int);

void abort(void);
void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *p, size_t size);
void free(void *ptr);

char *getenv(const char *name);

void __attribute__ ((noreturn)) exit(int status);

#endif /* __STDLIB_H__ */
