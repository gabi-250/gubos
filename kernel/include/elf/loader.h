#ifndef __ELF_LOADER_H__
#define __ELF_LOADER_H__

#include <elf/elf.h>
#include <mm/vmm.h>
#include <mm/paging.h>

void elf_load(paging_context_t kern_paging_ctx, vmm_context_t *kern_vmm_ctx, vmm_context_t *vmm_ctx,
              elf32_hdr_t header, void *raw_elf);

#endif /* __ELF_LOADER_H__ */
