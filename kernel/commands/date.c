#include "../libc.h"

void cmd_date(int argc, char **argv) { // date -> dd/mm/yy hh:mm:ss | date -t -> 176635....
    struct rtc_time t;
    rtc_read_time(&t);

    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        term_printf("%d\n", timestamp());
        return;
    }

    term_printf(
        "%d/%d/%d %d:%d:%d\n",
        t.day, t.month, t.year,
        t.hour, t.min, t.sec
    );
}