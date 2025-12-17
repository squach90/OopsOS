/*
   ___                   ___  ____
  / _ \  ___  _ __  ___ / _ \/ ___|
 | | | |/ _ \| '_ \/ __| | | \___ \
 | |_| | (_) | |_) \__ \ |_| |___) |
  \___/ \___/| .__/|___/\___/|____/
             |_|
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// List of all VGA Colors
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0, '*',0, ' '
};

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

size_t terminal_column;
size_t terminal_row;
int8_t terminal_color;
uint16_t* term_buffer = (uint16_t*)VGA_MEMORY;

size_t cursor_column;
size_t cursor_row;

int vga_entry_color(enum vga_color fg, enum vga_color bg) { // foreground color & background color
	return fg | (bg << 4);
}

int vga_entry(char character, uint8_t color) {
	return (uint16_t) character | (uint16_t) color << 8;
}

void term_init(void) {
	terminal_column = 0;
	terminal_row = 0;
	terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

	cursor_column = 0;
	cursor_row = 0;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {				// fill screen with ' '
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void term_clear(uint8_t fgColor, uint8_t bgColor) {
	terminal_color = vga_entry_color(fgColor, bgColor);
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void term_setcolor(uint8_t color) {
	terminal_color = color;
}

void term_put_entry_at(char character, uint8_t color, uint8_t x, uint8_t y) {
	const size_t index = y * VGA_WIDTH + x;
	term_buffer[index] = vga_entry(character, color);
}

static inline void update_cursor(size_t row, size_t column) {
    uint16_t pos = row * VGA_WIDTH + column;
    uint8_t low = pos & 0xFF;
    uint8_t high = (pos >> 8) & 0xFF;

    uint8_t command;

    command = 0x0F;
    asm volatile ("outb %0, %1" : : "a"(command), "Nd"(0x3D4));

    asm volatile ("outb %0, %1" : : "a"(low), "Nd"(0x3D5));

    command = 0x0E;
    asm volatile ("outb %0, %1" : : "a"(command), "Nd"(0x3D4));

    asm volatile ("outb %0, %1" : : "a"(high), "Nd"(0x3D5));
}


void term_putchar(char character) {
    if (character == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0; // or scroll
        }
        return;
    }

    term_put_entry_at(character, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0; // or scroll
        }
    }
	update_cursor(terminal_row, terminal_column);
}


void term_putchar_at(char c, size_t row, size_t col) {
    term_put_entry_at(c, terminal_color, col, row);
    update_cursor(row, col);
}

void term_writestring(const char* data)
{
    for (size_t i = 0; data[i] != '\0'; i++) {
        term_putchar(data[i]);
    }
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline bool keyboard_data_available(void) {
    // Port 0x64 is the Status Register.
    // Bit 0 (0x01) tells us if the Output Buffer is full (data ready).
    return inb(0x64) & 1;
}

void term_shell(void) {
    term_writestring("\n> "); // Initial prompt

    while (1) {
        if (keyboard_data_available()) {
            uint8_t scancode = inb(0x60);

            // Ignore key release events
            if (scancode & 0x80) continue;

            // HANDLE ENTER KEY (Scancode 0x1C)
            if (scancode == 0x1C) {
                term_writestring("\n> "); // Newline + Prompt
                continue; // Skip the rest of the loop
            }

            // Handle other characters
            if (scancode < 128 && scancode_to_ascii[scancode]) {
                char c = scancode_to_ascii[scancode];
                term_putchar(c);
            }
        }
    }
}

void kernel_main(void) {
    term_init();
    term_writestring(
        "   ___                   ___  ____\n"
        "  / _ \\  ___  _ __  ___ / _ \\/ ___|\n"
        " | | | |/ _ \\| '_ \\/ __| | | \\___ \\\n"
        " | |_| | (_) | |_) \\__ \\ |_| |___) |\n"
        "  \\___/ \\___/| .__/|___/\\___/|____/\n"
        "             |_|\n"
    );
    term_writestring("   OopsOS v0.? (c) 2025 squach90\n");
    term_writestring("   Press ENTER to start...");

    // Wait for Enter (28) to start the shell
    while (1) {
        if (keyboard_data_available()) {
            uint8_t scancode = inb(0x60);
            if (scancode == 28) { // Enter key
                term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                terminal_column = 0;
                terminal_row = 0;
                break;
            }
        }
    }
    term_shell();
}