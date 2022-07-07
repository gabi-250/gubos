#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>

typedef struct task_list {
    task_control_block_t *task;
    task_control_block_t *next;
} task_list_t;

void init_sched();
void sched_switch_task(task_control_block_t *);
void switch_to_user_mode(uint32_t);

#endif /* __SCHED_H__ */
