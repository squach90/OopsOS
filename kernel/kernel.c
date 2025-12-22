#include <stdbool.h>
#include "vga.h"
#include "keyboard.h"
#include "shell.h"
#include "libc.h"
#include "info.h"
#include "commands/commands.h"

void kernel_main(void) {
    // Init
    term_init();

    // Splash screen
    cmd_ascii(0, NULL);
    term_printf("   OopsOS v%s (c) 2025 squach90\n", kernel_version);
    term_printf("   Press ENTER to start...\n");

    // Wait for ENTER
    while (1) {
        if (keyboard_data_available()) {
            if (inb(0x60) == 0x1C) {
                break;
            }
        }
    }

    // Clear and start shell
    cmd_clear(0, NULL);

    term_shell();
}