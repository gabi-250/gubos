#include <stdint.h>
#include "multiboot2.h"
#include "printk.h"

void
multiboot_print_memory_map(struct multiboot_tag *tag, multiboot_memory_map_t *mmap) {
    printk("Memory map:\n");
    while ((multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size) {
        printk("    address: %lld\n", mmap->addr);
        printk("    length: %lld\n", mmap->len);
        printk("    type: ");
        switch (mmap->type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                printk("available\n");
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                printk("reserved\n");
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                printk("ACPI reclaimable\n");
                break;
            case MULTIBOOT_MEMORY_NVS:
                printk("NVS\n");
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                printk("bad RAM\n");
                break;
        }
        mmap = (multiboot_memory_map_t *)((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size);
    }
}

void
multiboot_print_info(uint32_t multiboot_info) {
    struct multiboot_tag *tag = (struct multiboot_tag *) (multiboot_info + 8);
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP:
                multiboot_print_memory_map(tag, ((struct multiboot_tag_mmap *) tag)->entries);
                break;
            default:
                break;
        }
       tag = (struct multiboot_tag *)((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7));
    }
}
