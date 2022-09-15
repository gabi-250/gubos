#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>
#include <stddef.h>

#define ELF_NIDENT 16
#define ELF_IDENT_MAGIC0     0
#define ELF_IDENT_MAGIC1     1
#define ELF_IDENT_MAGIC2     2
#define ELF_IDENT_MAGIC3     3
#define ELF_IDENT_CLASS      4
#define ELF_IDENT_DATA       5
#define ELF_IDENT_VERSION    6
#define ELF_IDENT_OSABI      7
#define ELF_IDENT_ABIVERSION 8

// Executable file
#define ELF_TYPE_EXEC 2

// Intel 80386
#define ELF_MACHINE_386 3

// ELF magic bytes
#define ELF_MAGIC0 0x7f
#define ELF_MAGIC1 0x45
#define ELF_MAGIC2 0x4c
#define ELF_MAGIC3 0x46

// 32-bit objects
#define ELF_CLASS32 1

// Little-endian
#define ELF_DATA_LSB 1

// No ABI-specific extensions or unspecified
#define ELF_OSABI_NONE 0

// Errors
#define ELF_PARSE_ERR_TRUNCATED 1
#define ELF_PARSE_ERR_INV_IDENT 2
#define ELF_PARSE_ERR_INV_TYPE 3
#define ELF_PARSE_ERR_INV_CLASS 4
#define ELF_PARSE_ERR_INV_DATA 5
#define ELF_PARSE_ERR_INV_OSABI 6
#define ELF_PARSE_ERR_INV_MACHINE 7
#define ELF_PARSE_ERR_INV_VERSION 8

// Program headers

// A loadable segment
#define ELF_PROG_HDR_TYPE_LOAD 1

// Executable segment
#define ELF_PROG_HDR_FLAG_X 1
// Writable segment
#define ELF_PROG_HDR_FLAG_W 2
// Readable segment
#define ELF_PROG_HDR_FLAG_R 4

typedef struct {
    uint8_t ident[ELF_NIDENT];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf32_hdr_t;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} elf32_prog_hdr_t;

int elf_parse_header(void *elf, size_t len, elf32_hdr_t *hdr);

#endif /* __ELF_H__ */
