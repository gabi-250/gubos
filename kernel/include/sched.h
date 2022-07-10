#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>

typedef enum sched_priority {
    SCHED_PRIORITY_LOW,
} sched_priority_t;

void init_sched();
void sched_switch_task(task_control_block_t *);
void sched_add(task_control_block_t *, sched_priority_t);
void sched_context_switch();
void switch_to_user_mode(uint32_t);

#endif /* __SCHED_H__ */
