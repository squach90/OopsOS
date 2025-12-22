#include "../libc.h"
#include "commands.h"

void cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    term_printf("Available commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        if (commands[i].description)  // only print if description exists
            term_printf("   %s - %s\n", commands[i].name, commands[i].description);
    }
}