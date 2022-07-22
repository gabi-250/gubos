#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>
#include <mm/paging.h>

typedef enum sched_priority {
    SCHED_PRIORITY_LOW,
} sched_priority_t;

void init_sched(paging_context_t);
void sched_add(task_control_block_t *, sched_priority_t);
void sched_remove(uint32_t pid);
void sched_context_switch();
void sched_halt_or_crash();

#endif /* __SCHED_H__ */
