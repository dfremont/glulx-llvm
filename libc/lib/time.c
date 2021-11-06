
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <clangglk.h>

struct tm _the_local_time;

time_t time(time_t *arg) {
	time_t t = glk_current_simple_time(1);
	if (arg) *arg = t;
	return t;
}

struct tm *localtime(const time_t *timer) {
	glkdate_t gd;
	glk_simple_time_to_date_local(*timer, 1, &gd);
	_the_local_time.tm_sec = gd.second;
	_the_local_time.tm_min = gd.minute;
	_the_local_time.tm_hour = gd.hour;
	_the_local_time.tm_mday = gd.day;
	_the_local_time.tm_mon = gd.month - 1;
	_the_local_time.tm_year = gd.year;
	_the_local_time.tm_wday = gd.weekday;
	unsigned yday = gd.day - 1;
	for (unsigned mon = gd.month-1; mon; --mon) {
		switch(mon) {
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			yday += 31;
			break;

			case 4:
			case 6:
			case 9:
			case 11:
			yday += 30;
			break;

			case 2:
			if (!(gd.year % 4) && (gd.year % 100) && !(gd.year % 400))
				yday += 29;
			else
				yday += 28;
			break;
		}
	}
	_the_local_time.tm_yday = yday;
	_the_local_time.tm_isdst = -1;		// indicates no information available
	return &_the_local_time;
}

size_t strftime(char *restrict str, size_t count,
                const char *restrict format, const struct tm *restrict time) {
	if (strcmp(format, "%y%m%d")) {
		assert(0 && "unsupported strftime format string");
		return 0;		// TODO handle other cases?
	}

	return snprintf(str, count, "%02d%02d%02d",
		time->tm_year % 100, time->tm_mon, time->tm_mday);
}
