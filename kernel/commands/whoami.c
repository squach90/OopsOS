#include "../info.h"

void cmd_whoami(int argc, char **argv) {
    (void)argc;
    (void)argv;
    term_printf("%s\n", username);
}