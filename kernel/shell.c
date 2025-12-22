#include "shell.h"
#include "vga.h"
#include "libc.h"
#include "keyboard.h"
#include "info.h"
#include "commands/commands.h"

#define CMD_HISTORY_SIZE 128

char last_command[CMD_HISTORY_SIZE] = {0};

char command_buffer[128];
int buffer_index = 0;   // cursor position
int buffer_length = 0;  // total chars in buffer


void print_prompt(void) {
    term_printf_color(VGA_COLOR_LIGHT_GREEN, username);
    term_putchar_color('@', VGA_COLOR_LIGHT_GREEN | (VGA_COLOR_BLACK << 4));
    term_printf_color(VGA_COLOR_LIGHT_GREEN, hostname);
    term_putchar_color(':', VGA_COLOR_LIGHT_GREY | (VGA_COLOR_BLACK << 4));
    term_printf_color(VGA_COLOR_LIGHT_BLUE, directory);

    term_putchar_color('$', VGA_COLOR_LIGHT_GREY | (VGA_COLOR_BLACK << 4));
    term_putchar(' ');
}

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

    term_printf("OopsOS: command not found: %s \n", argv[0]);
}

void term_shell(void) {
    char command_buffer[128];
    int buffer_index = 0;

    buffer_length = 0;
    print_prompt();

    while (1) {
        if (keyboard_data_available()) {
            uint8_t scancode = inb(0x60);

            // UP
            if (scancode == 0x48) {
                // clear line
                while (buffer_index > 0) {
                    buffer_index--;
                    terminal_column--;
                    term_putchar_at(' ', terminal_row, terminal_column);
                }

                // copy last command
                int len = strlen(last_command);
                for (int i = 0; i < len; i++) {
                    command_buffer[i] = last_command[i];
                    term_putchar(command_buffer[i]);
                }

                buffer_index = len; // cursor at end
                buffer_length = len; // full buffer
                continue;
            }

            // LEFT
            if (scancode == 0x4B && buffer_index > 0) {
                buffer_index--;
                terminal_column--;
                update_cursor(terminal_row, terminal_column);
                continue;
            }

            // RIGHT
            if (scancode == 0x4D && buffer_index < buffer_length) {
                buffer_index++;
                terminal_column++;
                update_cursor(terminal_row, terminal_column);
                continue;
            }

            char c = handle_scancode(scancode);
            if (!c) continue;

            if (c == '\n') {
                command_buffer[buffer_index] = '\0';
                term_putchar('\n');

                if (buffer_index > 0) {
                    strcpy(last_command, command_buffer); // save last cmd
                    execute_command(command_buffer);
                }

                buffer_index = 0;
                buffer_length = 0;
                print_prompt();

                continue;
            }

            if (c == '\b') {
                if (buffer_index > 0) {
                    buffer_index--;
                    terminal_column--;
                    term_putchar_at(' ', terminal_row, terminal_column);
                }
                continue;
            }

            if (buffer_index < 127) {
                command_buffer[buffer_index] = c;       // put char at cursor
                buffer_index++;                         // move cursor forward
                if (buffer_index > buffer_length)       // update buffer length if we added new char
                    buffer_length = buffer_index;
                term_putchar(c);                        // print char on screen
            }
        }
    }
}