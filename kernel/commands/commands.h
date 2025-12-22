#ifndef COMMANDS_H
#define COMMANDS_H

#include "../shell.h"

void cmd_ascii(int argc, char **argv);
void cmd_clear(int argc, char **argv);
void cmd_date(int argc, char **argv);
void cmd_echo(int argc, char* argv[]);
void cmd_help(int argc, char **argv);
void cmd_reboot(int argc, char **argv);
void cmd_setup(int argc, char **argv);
void cmd_shutdown(int argc, char **argv);
void cmd_version(int argc, char **argv);
void cmd_whoami(int argc, char **argv);

extern Command commands[];

#endif
