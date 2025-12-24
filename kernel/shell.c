#include "shell.h"
#include "vga.h"
#include "libc.h"
#include "keyboard.h"
#include "info.h"
#include "commands/commands.h"

#define MAX_HISTORY 50      // nb max of command in history
#define CMD_LEN 128

char history[MAX_HISTORY][CMD_LEN];
int history_count = 0;       // Total commands currently stored (caps at 50)
int history_write_idx = 0;   // Where the next command will be saved
int history_browse_idx = -1; // Where the user is currently looking (-1 = not browsing)

char command_buffer[128];
int buffer_index = 0;   // cursor position
int buffer_length = 0;  // total chars in buffer

void clear_current_line(int* index, int* length) {
    // Move cursor back to start of prompt
    while (*index > 0) {
        (*index)--;
        terminal_column--;
    }
    // Fill with spaces to clear existing text
    for (int i = 0; i < *length; i++) {
        term_putchar_at(' ', terminal_row, terminal_column + i);
    }
    *length = 0;
    update_cursor(terminal_row, terminal_column);
}

void print_prompt(void) { // user@hostname:directory$
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
            if (scancode == 0x48 && history_count > 0) {
                if (history_browse_idx == -1) {
                    history_browse_idx = (history_write_idx - 1 + MAX_HISTORY) % MAX_HISTORY;
                } else {
                    int oldest_idx = (history_count < MAX_HISTORY) ? 0 : history_write_idx;
                    if (history_browse_idx != oldest_idx) {
                        history_browse_idx = (history_browse_idx - 1 + MAX_HISTORY) % MAX_HISTORY;
                    }
                }

                clear_current_line(&buffer_index, &buffer_length);
                strcpy(command_buffer, history[history_browse_idx]);
                buffer_length = strlen(command_buffer);
                buffer_index = buffer_length;
                term_printf(command_buffer);
                continue;
            }

            // DOWN ARROW
            if (scancode == 0x50) {
                if (history_browse_idx != -1) {
                    // Move forward
                    history_browse_idx = (history_browse_idx + 1) % MAX_HISTORY;

                    if (history_browse_idx == history_write_idx) {
                        history_browse_idx = -1;
                        clear_current_line(&buffer_index, &buffer_length);
                        command_buffer[0] = '\0';
                    } else {
                        clear_current_line(&buffer_index, &buffer_length);
                        strcpy(command_buffer, history[history_browse_idx]);
                        buffer_length = strlen(command_buffer);
                        buffer_index = buffer_length;
                        term_printf(command_buffer);
                    }
                }
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
                command_buffer[buffer_length] = '\0';
                term_putchar('\n');

                if (buffer_length > 0) {
                    strcpy(history[history_write_idx], command_buffer);
                    history_write_idx = (history_write_idx + 1) % MAX_HISTORY;
                    if (history_count < MAX_HISTORY) history_count++;
                    history_browse_idx = -1;
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