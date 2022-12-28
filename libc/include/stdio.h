#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>
#include <stddef.h>

#define SEEK_SET 1

typedef struct file {
    int fd;
} FILE;

extern FILE *stderr;

FILE *fopen(const char *path, const char *mode);
FILE *fdopen(int fd, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *f);

int feof(FILE *f);
long ftell(FILE *f);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *f);
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *f);
int fseek(FILE *f, long offset, int whence);
int fclose(FILE *f);
int fflush (FILE *f);

int fprintf (FILE *f, const char *fmt, ...);

int vdprintf(int f, const char *format, va_list ap);
int vfprintf(FILE *f, const char *format, va_list ap);
int vprintf(const char *format, va_list ap);
int vsnprintf(char *s, size_t n, const char *format, va_list ap);
int vsprintf(char *s, const char *format, va_list ap);

int sprintf(char *s, const char *format, ...);

#endif /* __STDIO_H__ */
