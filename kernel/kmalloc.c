#include <stdint.h>
#include "kmalloc.h"

kmalloc_header_t *KMALLOC_HEAD;

void init_heap() {
    KMALLOC_HEAD = (kmalloc_header_t *)KERNEL_HEAP_START;
    *KMALLOC_HEAD = (kmalloc_header_t){
        .size = KERNEL_HEAP_SIZE - sizeof(kmalloc_header_t),
        .next = NULL,
    };
}

void * kmalloc(size_t size) {
    kmalloc_header_t *free_region = KMALLOC_HEAD;
    kmalloc_header_t *prev_region = NULL;

    while (free_region != NULL) {
        if (free_region->size == size) {
            // Return the entire block.
            if (prev_region == NULL) {
                KMALLOC_HEAD = KMALLOC_HEAD->next;
            } else {
                prev_region->next = free_region->next;
            }

            return (void *)((char *)free_region + sizeof(kmalloc_header_t));
        } else if (free_region->size > size + sizeof(kmalloc_header_t)) {
            // Split the block
            size_t remaining = free_region->size - size;
            kmalloc_header_t *new_region =
                (kmalloc_header_t *)((char *)free_region + sizeof(kmalloc_header_t) + size);

            *new_region = (kmalloc_header_t){
                .size = remaining - sizeof(kmalloc_header_t),
                .next = free_region->next,
            };

            if (prev_region == NULL) {
                KMALLOC_HEAD = new_region;
            } else {
                prev_region->next = new_region;
            }

            free_region->size = size;

            return (void *)((char *)free_region + sizeof(kmalloc_header_t));
        }
        prev_region = free_region;
        free_region = free_region->next;
    }

    // No suitably sized free block found.
    return NULL;
}

void kfree(void *addr) {
    kmalloc_header_t *region =
        (kmalloc_header_t *)((char *)addr - sizeof(kmalloc_header_t));

    region->next = KMALLOC_HEAD;
    KMALLOC_HEAD = region;
}
