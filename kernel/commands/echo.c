#include <stdint.h>
#include "../shell.h"
#include "../vga.h"
#include "../libc.h"

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

        if (i < argc - 1) {
            if (rainbowFlag) {
                term_putchar_color(' ', rainbowColors[colorIndex % colorCount] | (VGA_COLOR_BLACK << 4));
                colorIndex++;
            } else {
                term_putchar(' ');
            }
        }
    }


    term_printf("\n");
}