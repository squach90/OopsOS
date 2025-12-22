#include "vga.h"
#include "libc.h"
#include "keyboard.h"
#include <stdarg.h>

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
        terminal_row++;
    } else {
        term_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(character, terminal_color);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    scroll(); // scroll when row >= VGA_HEIGHT
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

void term_putchar_at_color(char c, size_t row, size_t col, uint8_t color) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= 25) terminal_row = 0;
        return;
    }

    if (row >= 25 || col >= 80) return;

    size_t index = row * 80 + col;
    uint16_t* vga = (uint16_t*)0xB8000;
    vga[index] = ((uint16_t)color << 8) | (uint8_t)c;

    update_cursor(row, col);
}


void term_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i]; i++) {
        if (fmt[i] == '%' && fmt[i + 1]) {
            i++;
            switch (fmt[i]) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    term_writestring(s);
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    char buf[16];
                    itoa(d, buf, 10);
                    term_writestring(buf);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    term_putchar(c);
                    break;
                }
                case '%':
                    term_putchar('%');
                    break;
                default:
                    term_putchar('%');
                    term_putchar(fmt[i]);
                    break;
            }
        } else {
            term_putchar(fmt[i]);
        }
    }

    va_end(args);
}

void term_printf_at(size_t row, size_t col, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t cur_row = row;
    size_t cur_col = col;

    for (size_t i = 0; fmt[i]; i++) {
        if (fmt[i] == '%' && fmt[i + 1]) {
            i++;
            switch (fmt[i]) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    for (size_t j = 0; s[j]; j++) {
                        term_putchar_at(s[j], cur_row, cur_col);
                        cur_col++;
                        if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                        if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    }
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    char buf[16];
                    itoa(d, buf, 10);
                    for (size_t j = 0; buf[j]; j++) {
                        term_putchar_at(buf[j], cur_row, cur_col);
                        cur_col++;
                        if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                        if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    term_putchar_at(c, cur_row, cur_col);
                    cur_col++;
                    if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                    if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    break;
                }
                case '%':
                    term_putchar_at('%', cur_row, cur_col);
                    cur_col++;
                    if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                    if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    break;
                default:
                    term_putchar_at('%', cur_row, cur_col);
                    cur_col++;
                    term_putchar_at(fmt[i], cur_row, cur_col);
                    cur_col++;
                    if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                    if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    break;
            }
        } else {
            term_putchar_at(fmt[i], cur_row, cur_col);
            cur_col++;
            if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
            if (cur_row >= VGA_HEIGHT) cur_row = 0;
        }
    }

    va_end(args);
}

void term_printf_at_color(size_t row, size_t col, uint8_t color, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t cur_row = row;
    size_t cur_col = col;

    for (size_t i = 0; fmt[i]; i++) {
        if (fmt[i] == '%' && fmt[i + 1]) {
            i++;
            switch (fmt[i]) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    for (size_t j = 0; s[j]; j++) {
                        term_putchar_at_color(s[j], cur_row, cur_col, color);
                        cur_col++;
                        if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                        if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    }
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    char buf[16];
                    itoa(d, buf, 10);
                    for (size_t j = 0; buf[j]; j++) {
                        term_putchar_at_color(buf[j], cur_row, cur_col, color);
                        cur_col++;
                        if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                        if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    term_putchar_at_color(c, cur_row, cur_col, color);
                    cur_col++;
                    if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                    if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    break;
                }
                case '%':
                    term_putchar_at_color('%', cur_row, cur_col, color);
                    cur_col++;
                    if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                    if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    break;
                default:
                    term_putchar_at_color('%', cur_row, cur_col, color);
                    cur_col++;
                    term_putchar_at_color(fmt[i], cur_row, cur_col, color);
                    cur_col++;
                    if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
                    if (cur_row >= VGA_HEIGHT) cur_row = 0;
                    break;
            }
        } else {
            term_putchar_at_color(fmt[i], cur_row, cur_col, color);
            cur_col++;
            if (cur_col >= VGA_WIDTH) { cur_col = 0; cur_row++; }
            if (cur_row >= VGA_HEIGHT) cur_row = 0;
        }
    }

    va_end(args);
}


char* term_readline(char* buffer, size_t max) {
    size_t i = 0;

    while (1) {
        if (!keyboard_data_available())
            continue;

        uint8_t scancode = inb(0x60);
        char c = handle_scancode(scancode);
        if (!c) continue;

        if (c == '\n') {
            term_putchar('\n');
            buffer[i] = '\0';
            return buffer;
        }

        if (c == '\b') {
            if (i > 0) {
                i--;
                terminal_column--;
                term_putchar_at(' ', terminal_row, terminal_column);
            }
            continue;
        }

        if (i < max - 1) {
            buffer[i++] = c;
            term_putchar(c);
        }
    }
}

void term_print_color(const char* str, uint8_t color) {
    for (size_t i = 0; str[i]; i++)
        term_putchar_color(str[i], color);
}

void term_printf_color(int fg, const char* s) {
    term_print_color(s, fg | (VGA_COLOR_BLACK << 4));
}

static inline uint16_t vga_get_char_at(size_t row, size_t col) {
    return term_buffer[row * VGA_WIDTH + col];
}

static inline void vga_set_char_at(size_t row, size_t col, uint16_t val) {
    term_buffer[row * VGA_WIDTH + col] = val;
}


void scroll(void) {
    if (terminal_row < VGA_HEIGHT)
        return;

    // shift all rows up by 1
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            uint16_t ch = vga_get_char_at(y, x);
            vga_set_char_at(y - 1, x, ch);
        }
    }

    // clear last row
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_set_char_at(VGA_HEIGHT - 1, x, ' ' | (VGA_COLOR_BLACK << 8));

    terminal_row = VGA_HEIGHT - 1;
}
