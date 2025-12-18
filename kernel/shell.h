#ifndef SHELL_H
#define SHELL_H

typedef void (*CommandHandler)(const char* args);

typedef struct {
    const char* name;
    const char* description;
    void (*handler)(int argc, char* argv[]);
} Command;

extern Command commands[];

void term_shell(void);
void execute_command(char* input);

#endif