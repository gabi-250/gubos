#ifndef __INIT_H__
#define __INIT_H__

#include <task.h>
#include <mm/vmm.h>

task_control_block_t *init_create_task0(paging_context_t paging_ctx, vmm_context_t vmm_ctx,
                                        void *text_physical_addr);

#endif /* __INIT_H__ */
