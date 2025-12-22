#include "vga.h"
#include "libc.h"
#include "keyboard.h"
#include "shell.h"

#define WIDTH 80
#define HEIGHT 20

static uint32_t seed;

int score = 0;
int highScore = 0;
int lines = 0;

void init_seed() {
    seed = timestamp(); // use timestamp to set seed
}

unsigned int rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void delay(int loops) {
    for (volatile int i = 0; i < loops; i++);
}

void game_dodge() {
    term_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    score = 0;
    lines = 0;

    char grid[HEIGHT][WIDTH];
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            grid[y][x] = ' ';

    int player_x = WIDTH / 2;
    int player_y = HEIGHT - 1;

    int tick = 0;
    int speed = 10;       // bigger = slower game
    int delay_time = 10000000;

    while (1) {
        // Handle player input
        static uint8_t extended = 0;

        if (keyboard_data_available()) {
            uint8_t scancode = inb(0x60);

            // Extended key prefix
            if (scancode == 0xE0) {
                extended = 1;
                continue;
            }

            char c = handle_scancode(scancode);

            // LEFT arrow
            if ((extended && scancode == 0x4B) || c == 'a') {
                if (player_x > 0) player_x--;
            }

            // RIGHT arrow
            if ((extended && scancode == 0x4D) || c == 'd') {
                if (player_x < WIDTH - 1) player_x++;
            }

            // Quit game
            if (c == 'q') {
                cmd_clear(0, NULL);
                break;
            }

            extended = 0; // reset after handling
        }


        // Move blocks every speed ticks
        if (tick % speed == 0) {
            // shift rows down
            for (int y = HEIGHT - 1; y > 0; y--)
                for (int x = 0; x < WIDTH; x++)
                    grid[y][x] = grid[y - 1][x];

            // new top row random blocks
            for (int x = 0; x < WIDTH; x++)
                grid[0][x] = (rand() % 10 == 0) ? '#' : ' ';

            lines++;
            if (lines >= 20) {
                score++;
            }
        }

        // Draw the grid
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                term_putchar_at(grid[y][x], y, x);

        // Draw the player
        term_putchar_at_color('@', player_y, player_x, VGA_COLOR_LIGHT_BLUE);

        term_printf_at_color(24, 0, VGA_COLOR_LIGHT_GREEN, "Score: %d", score);

        // Check collision
        if (grid[player_y][player_x] == '#') {
            cmd_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            score--;
            if (score >= highScore) {
                highScore = score;
            }
            term_printf_color(VGA_COLOR_LIGHT_RED ,"Game Over!\n");
            term_printf("Score: %d\n", score);
            term_printf("High Score: %d\n", highScore);
            break;
        }

        tick++;

        delay(delay_time);
    }
}
