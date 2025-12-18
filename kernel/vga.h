#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

enum vga_color {
    VGA_COLOR_BLACK = 0, VGA_COLOR_BLUE = 1, VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3, VGA_COLOR_RED = 4, VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6, VGA_COLOR_LIGHT_GREY = 7, VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9, VGA_COLOR_LIGHT_GREEN = 10, VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12, VGA_COLOR_LIGHT_MAGENTA = 13, VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

extern size_t terminal_column;
extern size_t terminal_row;
extern int8_t terminal_color;

void term_init(void);
void term_clear(uint8_t fg, uint8_t bg);
void term_putchar(char character);
void term_putchar_at(char c, size_t row, size_t col);
void term_writestring(const char* data);
void update_cursor(size_t row, size_t column);
void term_putchar_color(char c, uint8_t color);
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

#endif