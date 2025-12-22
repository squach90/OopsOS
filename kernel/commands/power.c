#include "../vga.h"
#include "commands.h"

void cmd_reboot(int argc, char **argv) {
    (void)argc;
    (void)argv;
    term_printf("Rebooting...\n");
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

void cmd_shutdown(int argc, char **argv) {
    (void)argc;
    (void)argv;
    term_printf("Shutting down...\n");
    cmd_clear(0, NULL);
    cmd_ascii(0, NULL);
    term_printf("\nYOU CAN NOW PRESS THE POWER BUTTON\n");

    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}