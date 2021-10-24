
#include <stddef.h>

size_t strlen(const char *str);

char *strcpy(char *restrict dest, const char *restrict src);

// Library functions which the backend will turn into @mcopy.
void* memcpy(void *restrict dest, const void *restrict src, size_t count);
void* memmove(void * dest, const void * src, size_t count);

// The backend will turn this into @mzero when possible.
void *memset(void * dest, int ch, size_t count);
