#include <elf/elf.h>
#include <string.h>

int
elf_parse_header(void *elf, size_t len, elf32_hdr_t *hdr) {
    if (len < sizeof(elf32_hdr_t)) {
        return ELF_PARSE_ERR_TRUNCATED;
    }

    memcpy(hdr, elf, sizeof(elf32_hdr_t));

    if (hdr->ident[ELF_IDENT_MAGIC0] != ELF_MAGIC0 || hdr->ident[ELF_IDENT_MAGIC1] != ELF_MAGIC1
            || hdr->ident[ELF_IDENT_MAGIC2] != ELF_MAGIC2 || hdr->ident[ELF_IDENT_MAGIC3] != ELF_MAGIC3) {
        return ELF_PARSE_ERR_INV_IDENT;
    }

    if (hdr->ident[ELF_IDENT_CLASS] != ELF_CLASS32) {
        return ELF_PARSE_ERR_INV_CLASS;
    }

    if (hdr->ident[ELF_IDENT_DATA] != ELF_DATA_LSB) {
        return ELF_PARSE_ERR_INV_DATA;
    }

    if (hdr->ident[ELF_IDENT_OSABI] != ELF_OSABI_NONE) {
        return ELF_PARSE_ERR_INV_OSABI;
    }

    if (hdr->type != ELF_TYPE_EXEC) {
        return ELF_PARSE_ERR_INV_TYPE;
    }

    if (hdr->machine != ELF_MACHINE_386) {
        return ELF_PARSE_ERR_INV_MACHINE;
    }

    if (hdr->machine != ELF_MACHINE_386) {
        return ELF_PARSE_ERR_INV_MACHINE;
    }

    return 0;
}
