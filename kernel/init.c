#include <init.h>
#include <mm/vmm.h>
#include <mm/addr_space.h>
#include <mm/paging.h>
#include <stddef.h>

void init_goto_user_mode();

task_control_block_t *
init_create_task0(paging_context_t kern_paging_ctx, vmm_context_t kern_vmm_ctx,
                  void *text_physical_addr) {
    void *user_text_vaddr = vmm_map_pages(&kern_vmm_ctx, 0, (uint32_t)text_physical_addr, 1,
                                          PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    task_control_block_t *task = task_create(kern_paging_ctx, kern_vmm_ctx, init_goto_user_mode,
                                 user_text_vaddr, true);

    vmm_context_t vmm_context = vmm_clone_context(kern_vmm_ctx);

    vmm_map_pages(&vmm_context,
                  USER_STACK_TOP - USER_STACK_SIZE, 0, USER_STACK_PAGE_COUNT,
                  PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);


    task->vmm_context = vmm_context;

    return task;
}
