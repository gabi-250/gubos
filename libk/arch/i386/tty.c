#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include <tty.h>
#include <vga.h>

#include <multiboot2.h>

extern multiboot_info_t MULTIBOOT_INFO;

// These are all initialized by tty_init:
static size_t tty_row;
static size_t tty_column;
static uint8_t default_tty_color;
static uint16_t *vga_memory;
static uint32_t framebuf_height;
static uint32_t framebuf_width;

static void
tty_adjust_row() {
    if (++tty_row == framebuf_height) {
        for (size_t y = 0; y < framebuf_height - 1; y++) {
            for (size_t x = 0; x < framebuf_width; x++) {
                const size_t index_old = y * framebuf_width + x;
                const size_t index_new = (y + 1) * framebuf_width + x;
                vga_memory[index_old] = vga_memory[index_new];
            }
        }
        // Clear the last line
        for (size_t x = 0; x < framebuf_width; x++) {
            const size_t index = (framebuf_height - 1) * framebuf_width + x;
            vga_memory[index] = vga_entry(' ', default_tty_color);
        }
        tty_row = framebuf_height - 1;
    }
}

static void
tty_adjust_indices() {
    if (++tty_column == framebuf_width) {
        tty_column = 0;
        tty_adjust_row();
    }
}

static void
tty_putchar(char c, uint8_t tty_color) {
    const size_t index = tty_row * framebuf_width + tty_column;
    vga_memory[index] = vga_entry((unsigned char)c, tty_color);
    tty_adjust_indices();
}

void
tty_init(uint32_t higher_half_base) {
    struct multiboot_tag_framebuffer_common *framebuffer_info =
        multiboot_framebuffer_info(MULTIBOOT_INFO.addr);

    framebuf_height = framebuffer_info->framebuffer_height;
    framebuf_width = framebuffer_info->framebuffer_width;

    vga_memory =
        (uint16_t *)((uint32_t)framebuffer_info->framebuffer_addr + higher_half_base);
    tty_row = 0;
    tty_column = 0;
    // grey text on a beautiful blue background
    default_tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_LIGHT_BLUE);
    for (size_t y = 0; y < framebuf_height; y++) {
        for (size_t x = 0; x < framebuf_width; x++) {
            const size_t index = y * framebuf_width + x;
            vga_memory[index] = vga_entry(' ', default_tty_color);
        }
    }
}

void
tty_puts(const char *str, size_t size, uint8_t tty_color) {
    for (size_t i = 0; i < size; i++) {
        if (str[i] == '\n') {
            tty_adjust_row();
            tty_column = 0;
        } else {
            tty_putchar(str[i], tty_color);
        }
    }
}
