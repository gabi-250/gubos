#ifndef __SCHED_H__
#define __SCHED_H__

#include "task.h"
#include "kernel_meminfo.h"

void init_sched(kernel_meminfo_t meminfo);
void sched_switch_task(task_control_block_t *);

#endif /* __SCHED_H__ */
