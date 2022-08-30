#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>
#include <stdbool.h>

int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
size_t strlen(const char *);
char *strrev(char *);
char *strupper(char *s);
bool islower(char);
char toupper(char);

#endif /* __STRING_H__ */
