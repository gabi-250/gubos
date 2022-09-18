#include <elf/elf.h>
#include <elf/loader.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <panic.h>
#include <string.h>

static uint32_t
elf_flags_to_paging_flags(uint32_t flags) {
    uint32_t pt_flags = 0;

    // TODO: handle other flags?
    if (flags & ELF_PROG_HDR_FLAG_W) {
        pt_flags |= PAGE_FLAG_WRITE;
    }

    return pt_flags;
}

static void
handle_loadable_segment(vmm_context_t *kern_vmm_context, vmm_context_t *vmm_context,
                        elf32_prog_hdr_t *prog_hdr, void *raw_elf) {
    size_t size = prog_hdr->filesz > prog_hdr->memsz ? prog_hdr->filesz : prog_hdr->memsz;
    size_t file_size = prog_hdr->filesz;
    size_t page_count = paging_page_count(size);

    for (size_t i = 0; i < page_count; ++i) {
        uint32_t virtual_addr = prog_hdr->vaddr + i * PAGE_SIZE;
        uint32_t aligned_vaddr = paging_align_addr(virtual_addr);
        uint32_t physical_addr = (uint32_t)pmm_alloc_page();

        uint32_t flags = PAGE_FLAG_PRESENT | PAGE_FLAG_USER | elf_flags_to_paging_flags(prog_hdr->flags);
        // Map the virtual address so we can memcpy the data from the ELF file
        // into the newly allocated page.
        vmm_map_pages(kern_vmm_context, aligned_vaddr, physical_addr, 1, flags);
        memcpy((void *)virtual_addr, (char *)raw_elf + prog_hdr->offset + i * PAGE_SIZE, file_size);
        vmm_unmap_pages(kern_vmm_context, aligned_vaddr, 1);

        // Add the same virtual address mapping into the context of the new
        // task.
        vmm_map_pages(vmm_context, aligned_vaddr, physical_addr, 1, flags);

        if (file_size < PAGE_SIZE) {
            // If the memory size is greater than the file size of the segment, the
            // extra bytes need to be set to 0.
            if (prog_hdr->memsz > prog_hdr->filesz) {
                memset((void *)(virtual_addr + file_size), 0, prog_hdr->memsz - prog_hdr->filesz);
            }
        } else {
            file_size -= PAGE_SIZE;
        }
    }
}

void
elf_load(vmm_context_t *kern_vmm_context, vmm_context_t *vmm_context, elf32_hdr_t header,
         void *raw_elf) {
    for (size_t i = 0; i < header.phnum; ++i) {
        size_t prog_header_offset = header.phoff + i * header.phentsize;
        elf32_prog_hdr_t *prog_hdr = (elf32_prog_hdr_t *)(raw_elf + prog_header_offset);

        switch (prog_hdr->type) {
            case ELF_PROG_HDR_TYPE_NULL:
                // Nothing to do.
                return;
            case ELF_PROG_HDR_TYPE_LOAD:
                handle_loadable_segment(kern_vmm_context, vmm_context, prog_hdr, raw_elf);

                break;
            default:
                PANIC("unsupported program header type %d", prog_hdr->type);
        }
    }
}
