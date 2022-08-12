#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>
#include <mm/vmm.h>
#include <mm/paging.h>

void init_sched(paging_context_t paging_ctx, vmm_context_t vmm_context);
void sched_add(task_control_block_t *, task_priority_t);
void sched_remove(uint32_t pid);
void sched_context_switch();
void sched_halt_or_crash();

#endif /* __SCHED_H__ */
