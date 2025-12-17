#include <stdbool.h>
#include "libc.h"
#include "vga.h"

char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0, '*',0, ' '
};

void sys_reboot(void) {
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

void sys_shutdown(void) {
    // Only QEMU
    outw(0x604, 0x2000);

    term_writestring("\nShutdown failed.\n");
}

static inline bool keyboard_data_available(void) {
    return inb(0x64) & 1;
}

void term_shell(void) {
    char command_buffer[128];
    int buffer_index = 0;
    term_writestring("\n> ");

    while (1) {
        if (keyboard_data_available()) {
            uint8_t scancode = inb(0x60);
            if (scancode & 0x80) continue;

            if (scancode == 0x1C) { // ENTER
                command_buffer[buffer_index] = '\0';
                term_putchar('\n');
                if (strcmp(command_buffer, "reboot") == 0) {
                    term_writestring("Reboot...");
                    sys_reboot();
                } else if (strcmp(command_buffer, "shutdown") == 0) {
                    term_writestring("Shutting down...");
                    sys_shutdown();
                }
                buffer_index = 0;
                term_writestring("> ");
                continue;
            }

            if (scancode == 0x0E) { // BACKSPACE
                if (buffer_index > 0) {
                    buffer_index--;
                    terminal_column--;
                    term_putchar_at(' ', terminal_row, terminal_column);
                }
                continue;
            }

            if (scancode < 128 && scancode_to_ascii[scancode]) {
                char c = scancode_to_ascii[scancode];
                if (buffer_index < 127) {
                    command_buffer[buffer_index++] = c;
                    term_putchar(c);
                }
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
    term_writestring("   OopsOS v0.? (c) 2025 squach90\n   Press ENTER to start...");

    while (1) {
        if (keyboard_data_available()) {
            if (inb(0x60) == 0x1C) {
                term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                terminal_column = 0; terminal_row = 0;
                update_cursor(0, 0);
                break;
            }
        }
    }
    term_shell();
}