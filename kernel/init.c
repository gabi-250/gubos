#include <init.h>
#include <mm/vmm.h>
#include <mm/addr_space.h>
#include <stddef.h>

extern vmm_context_t VMM_CONTEXT;

void init_goto_user_mode(uint32_t);

task_control_block_t *
init_create_task0(void *text_physical_addr) {
    vmm_map_pages(&VMM_CONTEXT,
                  USER_STACK_TOP - USER_STACK_SIZE, 0, USER_STACK_PAGE_COUNT,
                  PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

    /*void *virtual_addr =*/ vmm_map_pages(&VMM_CONTEXT, 0, (uint32_t)text_physical_addr, 1,
                                           PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);


    return NULL;
}
