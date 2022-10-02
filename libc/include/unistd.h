#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdint.h>

typedef uint32_t pid_t;

pid_t getpid(void);
pid_t fork(void);

int execve(const char *pathname, char *const argv[], char *const envp[]);
int execv(const char *pathname, char *const argv[]);
int execvp(const char *file, char *const argv[]);

#endif /* __UNISTD_H__ */
