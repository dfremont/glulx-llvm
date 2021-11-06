
typedef int time_t;

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

time_t time(time_t *arg);
struct tm *localtime(const time_t *timer);

size_t strftime(char *restrict str, size_t count,
                const char *restrict format, const struct tm *restrict time);
