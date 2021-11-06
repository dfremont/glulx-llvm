
#include <stdio.h>

void _assert(int condition, const char *cond_str,
	const char *func, const char *file, int line) {
	if (condition) return;
	const char fmt[] = "assertion \"%s\" (in function %s) on line %d of %s failed";
	fprintf(stderr, fmt, cond_str, func, line, file);
	__quit();
}
