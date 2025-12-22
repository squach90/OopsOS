#include "../vga.h"

void cmd_clear(int argc, char **argv) {
    (void)argc;
    (void)argv;
    term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_column = 0;
    terminal_row = 0;
}