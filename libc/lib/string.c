
#include <string.h>

size_t strlen(const char *str) {
	size_t i = 0;
    while (*p++)
    	++i;
    return i;
}

char *strcpy( char *restrict dest, const char *restrict src ) {
	while ((*dest++ = *src++))
		;
	return dest;
}

void *memset(void * dest, int ch, size_t count) {
	void * out = dest;
	while (count--)
		*dest++ = ch;
	return out;
}
