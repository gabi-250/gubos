#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <registers.h>
#include <interrupts/handlers.h>

#define SYS_EXIT 1

void syscall_handler(interrupt_state_t *, registers_t);

#endif /* __SYSCALL_H__ */
