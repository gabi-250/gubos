#include <init.h>
#include <task.h>
#include <panic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/addr_space.h>
#include <mm/paging.h>
#include <stddef.h>
#include <elf.h>

void init_goto_user_mode();

static void
load_elf(vmm_context_t *vmm_context, elf32_hdr_t header, void *raw_elf,
         void *raw_elf_physical_addr) {
    for (size_t i = 0; i < header.phnum; ++i) {
        size_t offset_in_file = header.phoff + i * header.phentsize;
        elf32_prog_hdr_t *prog_hdr = (elf32_prog_hdr_t *)(raw_elf + offset_in_file);

        // TODO: calculate how many pages to load
        size_t page_count = 1;
        uint32_t physical_addr = (uint32_t)raw_elf_physical_addr + header.phoff + offset_in_file;

        if (prog_hdr->type == ELF_PROG_HDR_TYPE_LOAD) {
            // TODO: allocate some physical pages and copy over the relevant
            // bits of the ELF binary instead of reusing the memory where GRUB
            // loaded the binary.
            vmm_map_pages(vmm_context, prog_hdr->vaddr, physical_addr, page_count,
                          PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

        }
    }
}

task_control_block_t *
init_create_task0(paging_context_t kern_paging_ctx, vmm_context_t kern_vmm_ctx,
                  void *user_elf_physical_addr) {
    task_control_block_t *task = init_create_user_task(kern_paging_ctx, kern_vmm_ctx,
                                 user_elf_physical_addr, NULL);

    ASSERT(task->pid == INIT_PID, "invalid PID for init task: %u", task->pid);

    return task;
}

task_control_block_t *
init_create_user_task(paging_context_t kern_paging_ctx, vmm_context_t kern_vmm_ctx,
                      void *user_elf_physical_addr, task_control_block_t *parent) {
    void *user_elf = vmm_map_pages(&kern_vmm_ctx, 0, (uint32_t)user_elf_physical_addr, 1,
                                   PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

    elf32_hdr_t header;
    // TODO use the real "file" length.
    size_t file_len = 1024;

    int res = elf_parse_header(user_elf, file_len, &header);

    if (res) {
        PANIC("failed to parse ELF: %d %u\n", res, header.type);
    }

    task_control_block_t *task = task_create(kern_paging_ctx, kern_vmm_ctx, init_goto_user_mode,
                                 (void *)header.entry, true);
    task->parent = parent;

    vmm_context_t vmm_context = vmm_clone_context(kern_vmm_ctx);
    load_elf(&vmm_context, header, user_elf, user_elf_physical_addr);

    for (size_t i = 0; i < USER_STACK_PAGE_COUNT; ++i) {
        uint32_t physical_addr = (uint32_t)pmm_alloc_page();
        vmm_map_pages(&vmm_context,
                      USER_STACK_TOP - USER_STACK_SIZE + i * PAGE_SIZE, physical_addr, 1,
                      PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    task->vmm_context = vmm_context;

    return task;

}
