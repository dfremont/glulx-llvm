
#include <stddef.h>

size_t strlen( const char *str );
size_t strnlen( const char *str, size_t strsz );
int strcmp( const char *lhs, const char *rhs );
int strncmp(register const char *s1, register const char *s2, size_t n);
char *strchr( const char *str, int ch );

char *strcpy( char *restrict dest, const char *restrict src );
char *strncpy( char *restrict dest, const char *restrict src, size_t count );
char *strcat( char *restrict dest, const char *restrict src );

int atoi( const char *str );
long strtol( const char *restrict str, char **restrict str_end, int base );

int memcmp( const void* lhs, const void* rhs, size_t count );
void* memcpy( void *restrict dest, const void *restrict src, size_t count );
void* memmove( void* dest, const void* src, size_t count );
void *memset( void *dest, int ch, size_t count );
