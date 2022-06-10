#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>

void init_sched();
void sched_switch_task(task_control_block_t *);

#endif /* __SCHED_H__ */
