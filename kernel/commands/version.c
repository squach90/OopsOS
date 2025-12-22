#include "../info.h"

void cmd_version(int argc, char **argv) {
    (void)argc;
    (void)argv;
    term_printf("OopsOS - Version: %s\n", kernel_version);
}