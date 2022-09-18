#ifndef __ELF_LOADER_H__
#define __ELF_LOADER_H__

#include <elf/elf.h>
#include <mm/vmm.h>

void elf_load(vmm_context_t *kern_vmm_context, vmm_context_t *vmm_context, elf32_hdr_t header,
              void *raw_elf);

#endif /* __ELF_LOADER_H__ */
