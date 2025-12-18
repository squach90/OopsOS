#include "vga.h"
#include "libc.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

size_t terminal_column;
size_t terminal_row;
int8_t terminal_color;
uint16_t* term_buffer = (uint16_t*)VGA_MEMORY;

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

static uint16_t vga_entry(char character, uint8_t color) {
    return (uint16_t) character | (uint16_t) color << 8;
}

void update_cursor(size_t row, size_t column) {
    uint16_t pos = row * VGA_WIDTH + column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void term_init(void) {
    terminal_column = 0;
    terminal_row = 0;
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void term_clear(uint8_t fg, uint8_t bg) {
    terminal_color = vga_entry_color(fg, bg);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        term_buffer[i] = vga_entry(' ', terminal_color);
    }
}

void term_putchar(char character) {
    if (character == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) terminal_row = 0;
    } else {
        term_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(character, terminal_color);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) terminal_row = 0;
        }
    }
    update_cursor(terminal_row, terminal_column);
}

void term_putchar_at(char c, size_t row, size_t col) {
    term_buffer[row * VGA_WIDTH + col] = vga_entry(c, terminal_color);
    update_cursor(row, col);
}

void term_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) term_putchar(data[i]);
}

void term_putchar_color(char c, uint8_t color) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= 25) terminal_row = 0; // simple scroll wrap
        return;
    }

    size_t index = terminal_row * 80 + terminal_column;
    uint16_t* vga = (uint16_t*)0xB8000;
    vga[index] = (uint16_t)color << 8 | (uint8_t)c;

    terminal_column++;
    if (terminal_column >= 80) {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= 25) terminal_row = 0;
    }
}
