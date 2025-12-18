#include <stdbool.h>
#include "vga.h"
#include "keyboard.h"
#include "shell.h"
#include "libc.h"

void print_logo(void) {
    term_writestring(
        "   ___                   ___  ____\n"
        "  / _ \\  ___  _ __  ___ / _ \\/ ___|\n"
        " | | | |/ _ \\| '_ \\/ __| | | \\___ \\\n"
        " | |_| | (_) | |_) \\__ \\ |_| |___) |\n"
        "  \\___/ \\___/| .__/|___/\\___/|____/\n"
        "             |_|\n"
    );
}

void kernel_main(void) {
    // Init
    term_init();

    // Splash screen
    print_logo();
    term_writestring("   OopsOS v0.? (c) 2025 squach90\n");
    term_writestring("   Press ENTER to start...\n");

    // Wait for ENTER
    while (1) {
        if (keyboard_data_available()) {
            if (inb(0x60) == 0x1C) {
                break;
            }
        }
    }

    // Clear and start shell
    term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_column = 0;
    terminal_row = 0;
    update_cursor(0, 0);

    term_shell();
}