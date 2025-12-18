#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

extern char scancode_to_ascii[128];

bool keyboard_data_available(void);

#endif