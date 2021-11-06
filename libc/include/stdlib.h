
#include <stddef.h>

_Noreturn void exit( int exit_code );

void* malloc( size_t size );
void *calloc( size_t num, size_t size );
void *realloc( void *ptr, size_t new_size );
void free( void* ptr );


void qsort( void *ptr, size_t count, size_t size,
            int (*comp)(const void *, const void *) );