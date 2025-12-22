#include "commands.h"
#include "../libc.h"

Command commands[] = {
    {"ascii",   "Show OopsOS logo",               cmd_ascii},
    {"clear",   "Clear the screen",               cmd_clear},
    {"date",    "Show current date/time (UTC)",   cmd_date},
    {"dodger",  NULL,                             game_dodge},
    {"echo",    "Print something",                cmd_echo},
    {"help",    "Show this help",                 cmd_help},
    {"reboot",  "Reboot the system",              cmd_reboot},
    {"setup",   "Setup all",                      cmd_setup},
    {"shutdown","Shutdown the system",            cmd_shutdown},
    {"version", "Show version of the kernel",     cmd_version},
    {"whoami",  "Show current user",              cmd_whoami},
    {NULL, NULL, NULL}
};
