#include "shell.h"
#include "vga.h"
#include "libc.h"
#include "keyboard.h"

void cmd_reboot() {
    term_writestring("Rebooting...\n");
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

void cmd_shutdown() {
    term_writestring("Shutting down...\n");
    term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_column = 0;
    terminal_row = 0;
    term_writestring(
        "   ___                   ___  ____\n"
        "  / _ \\  ___  _ __  ___ / _ \\/ ___|\n"
        " | | | |/ _ \\| '_ \\/ __| | | \\___ \\\n"
        " | |_| | (_) | |_) \\__ \\ |_| |___) |\n"
        "  \\___/ \\___/| .__/|___/\\___/|____/\n"
        "             |_|\n"
    );
    term_writestring("\nYOU CAN NOW PRESS THE POWER BUTTON\n");

    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

void cmd_echo(int argc, char* argv[]) {
    int rainbowFlag = 0;

    for (int i = 1; argv[i]; i++) {
        if (strcmp(argv[i], "-rainbow") == 0) {
            rainbowFlag = 1;
            break;
        }
    }

    uint8_t rainbowColors[] = {
        0x04, // red
        0x0C, // light red
        0x0E, // yellow
        0x02, // green
        0x03, // cyan
        0x09, // blue
        0x05, // magenta
        0x0D  // light magenta
    };

    int colorCount = sizeof(rainbowColors)/sizeof(rainbowColors[0]);
    int colorIndex = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-rainbow") == 0) continue;

        char* arg = argv[i];
        for (int j = 0; arg[j]; j++) {
            char c = arg[j];
            if (rainbowFlag) {
                term_putchar_color(c, rainbowColors[colorIndex % colorCount] | (VGA_COLOR_BLACK << 4));
                colorIndex++;
            } else {
                term_putchar(c);
            }
        }
    }

    term_writestring("\n");
}


void cmd_clear() {
    term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_column = 0;
    terminal_row = 0;
}

void cmd_help() {
    term_writestring("Available commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        term_writestring("  ");
        term_writestring(commands[i].name);
        term_writestring(" - ");
        term_writestring(commands[i].description);
        term_writestring("\n");
    }
}

// === COMMAND TABLE ===

Command commands[] = {
    {"clear", "Clear the screen",               cmd_clear},
    {"echo", "Print something one the screen",  cmd_echo},
    {"help", "Show this help",                  cmd_help},
    {"reboot",   "Reboot the system",           cmd_reboot},
    {"shutdown", "Shutdown the system",         cmd_shutdown},
    {NULL, NULL, NULL}
};


void execute_command(char* input) {
    char* argv[16];
    int argc = 0;

    // Split input by spaces
    char* p = input;
    while (*p && argc < 15) {
        // Skip spaces
        while (*p == ' ') p++;
        if (!*p) break;

        argv[argc++] = p;

        // Find end of token
        while (*p && *p != ' ') p++;
        if (*p) {
            *p = '\0';
            p++;
        }
    }
    argv[argc] = NULL;

    if (argc == 0) return;

    // Find and execute command
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv); // pass full argc/argv
            return;
        }
    }

    term_writestring("Unknown command: ");
    term_writestring(argv[0]);
    term_writestring("\n");
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

                if (buffer_index > 0) {
                    execute_command(command_buffer);
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