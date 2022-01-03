#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include <tty.h>
#include <vga.h>

#define VGA_HEIGHT 25
#define VGA_WIDTH 80
#define VGA_MEMORY (uint16_t*)(0xb8000 + 0xC0000000)

static size_t tty_row;
static size_t tty_column;
static uint8_t default_tty_color;
static uint16_t *vga_memory = VGA_MEMORY;

static void
tty_adjust_row() {
    if (++tty_row == VGA_HEIGHT) {
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t index_old = y * VGA_WIDTH + x;
                const size_t index_new = (y + 1) * VGA_WIDTH + x;
                vga_memory[index_old] = vga_memory[index_new];
            }
        }
        // Clear the last line
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            vga_memory[index] = vga_entry(' ', default_tty_color);
        }
        tty_row = VGA_HEIGHT - 1;
    }
}

static void
tty_adjust_indices() {
    if (++tty_column == VGA_WIDTH) {
        tty_column = 0;
        tty_adjust_row();
    }
}

static void
tty_putchar(char c, uint8_t tty_color) {
    const size_t index = tty_row * VGA_WIDTH + tty_column;
    vga_memory[index] = vga_entry((unsigned char)c, tty_color);
    tty_adjust_indices();
}

void
tty_init() {
    tty_row = 0;
    tty_column = 0;
    // grey text on a beautiful blue background
    default_tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_LIGHT_BLUE);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
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
