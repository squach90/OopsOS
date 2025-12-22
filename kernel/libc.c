#include "libc.h"

int strcmp(const char* s1, const char* s2) {        // ret 0 if s1 & s2 -> same
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}


size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

char* itoa(int value, char* buffer, int base) {
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    int tmp_value;

    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    int sign = 0;
    if (value < 0 && base == 10) {
        sign = 1;
        value = -value;
    }

    while (value) {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * base];
    }

    if (sign) {
        *ptr++ = '-';
    }

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return buffer;
}

int atoi(const char* s) {
    int result = 0;
    int sign = 1;

    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return result * sign;
}

int rtc_is_updating() {
    outb(0x70, 0x0A);
    return inb(0x71) & 0x80;
}

uint8_t rtc_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val / 16) * 10);
}

void rtc_read_time(struct rtc_time* t) {
    // wait until RTC not updating
    while (rtc_is_updating());

    uint8_t sec   = rtc_read(0x00);
    uint8_t min   = rtc_read(0x02);
    uint8_t hour  = rtc_read(0x04);
    uint8_t day   = rtc_read(0x07);
    uint8_t month = rtc_read(0x08);
    uint8_t year  = rtc_read(0x09);
    uint8_t regB  = rtc_read(0x0B);

    // convert if BCD
    if (!(regB & 0x04)) {
        sec   = bcd_to_bin(sec);
        min   = bcd_to_bin(min);
        hour  = bcd_to_bin(hour);
        day   = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year  = bcd_to_bin(year);
    }

    t->sec   = sec;
    t->min   = min;
    t->hour  = hour;
    t->day   = day;
    t->month = month;
    t->year  = 2000 + year;
}

int is_leap_year(int year) {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

// Convert RTC time to real Unix timestamp
uint32_t timestamp() {
    struct rtc_time t;
    rtc_read_time(&t);

    // Number of days per month
    const int days_in_month[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

    uint32_t days = 0;

    // Add days for all years since 1970
    for (int y = 1970; y < t.year; y++) {
        days += 365 + is_leap_year(y);
    }

    // Add days for months this year
    for (int m = 1; m < t.month; m++) {
        days += days_in_month[m-1];
        if (m == 2 && is_leap_year(t.year)) days++; // Feb leap day
    }

    // Add days this month
    days += t.day - 1;

    uint32_t seconds = days*86400 + t.hour*3600 + t.min*60 + t.sec;
    return seconds;
}
