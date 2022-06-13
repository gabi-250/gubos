#include <stdint.h>
#include <multiboot2.h>
#include <printk.h>

void
multiboot_print_memory_map(struct multiboot_tag *tag, multiboot_memory_map_t *mmap) {
    printk_debug("Memory map:\n");
    while ((multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size) {
        printk_debug("    address: %#llx | length: %llu | type: ", mmap->addr, mmap->len);
        switch (mmap->type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                printk_debug("available\n");
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                printk_debug("reserved\n");
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                printk_debug("ACPI reclaimable\n");
                break;
            case MULTIBOOT_MEMORY_NVS:
                printk_debug("NVS\n");
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                printk_debug("bad RAM\n");
                break;
        }

        multiboot_uint32_t entry_size = ((struct multiboot_tag_mmap *) tag)->entry_size;
        mmap = (multiboot_memory_map_t *)((unsigned long) mmap + entry_size);
    }
}

void
multiboot_print_framebuffer_info(struct multiboot_tag_framebuffer_common common) {
    printk_debug("Framebuffer: addr=%#llx size=%u width=%u height=%u\n",
                 common.framebuffer_addr, common.size, common.framebuffer_width,
                 common.framebuffer_height);
}

void
multiboot_print_info(uint32_t multiboot_info) {
    struct multiboot_tag *tag = (struct multiboot_tag *) (multiboot_info + 8);
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP:
                multiboot_print_memory_map(tag, ((struct multiboot_tag_mmap *) tag)->entries);
                break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                multiboot_print_framebuffer_info(((struct multiboot_tag_framebuffer *) tag)->common);
            default:
                break;
        }
        tag = (struct multiboot_tag *)((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7));
    }
}

struct multiboot_tag_framebuffer_common *
multiboot_framebuffer_info(uint32_t multiboot_info) {
    struct multiboot_tag *tag = (struct multiboot_tag *) (multiboot_info + 8);
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                return &((struct multiboot_tag_framebuffer *) tag)->common;
            default:
                break;
        }
        tag = (struct multiboot_tag *)((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7));
    }
    return 0;
}

uint32_t
multiboot_get_first_module(uint32_t multiboot_info) {
    struct multiboot_tag *tag = (struct multiboot_tag *) (multiboot_info + 8);
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
            return ((struct multiboot_tag_module *) tag)->mod_start;
        }
        tag = (struct multiboot_tag *)((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7));
    }
    return 0;
}
