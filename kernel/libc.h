#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>
#include <stddef.h>

struct rtc_time {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

int strcmp(const char* s1, const char* s2);
size_t strlen(const char* str);

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
int atoi(const char* s);
char* itoa(int value, char* buffer, int base);
int rtc_is_updating();
uint8_t rtc_read(uint8_t reg);
uint8_t bcd_to_bin(uint8_t val);
void rtc_read_time(struct rtc_time* t);
void strcpy(char* dest, const char* src);
uint32_t timestamp();
unsigned int rand(void);
void sleep(uint32_t count);

#endif