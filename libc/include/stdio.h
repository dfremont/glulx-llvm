
#include <stddef.h>
#include <stdarg.h>

#include <clangglk.h>

struct fpos_ty {
    size_t index;
};

typedef struct fpos_ty fpos_t;

struct file_ty {
	int is_glk;
    union {
        char *buffer;
        strid_t stream;
    };
    size_t maxsize;

    size_t pos;
    int error;
};

typedef struct file_ty FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define EOF (-1)

#define SEEK_SET seekmode_Start
#define SEEK_CUR seekmode_Current
#define SEEK_END seekmode_End

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_ACCMODE 3

FILE *fopen( const char *restrict filename, const char *restrict mode );
int fclose( FILE *stream );
int fflush( FILE *stream );

int feof( FILE *stream );
int ferror( FILE *stream );

int fseek( FILE *stream, long offset, int origin );
int fsetpos( FILE *stream, const fpos_t *pos );
int fgetpos( FILE *restrict stream, fpos_t *restrict pos );

size_t fread( void *restrict buffer, size_t size, size_t count,
              FILE *restrict stream );
size_t fwrite( const void *restrict buffer, size_t size, size_t count,
               FILE *restrict stream );
char *fgets( char *restrict str, int count, FILE *restrict stream );
int fputs( const char *restrict str, FILE *restrict stream );
int fgetc( FILE *stream );
int fputc( int ch, FILE *stream );

int remove( const char *fname );

int putchar( int ch );
int puts( const char *str );

int printf( const char *restrict format, ... );
int fprintf(FILE * __restrict stream, const char * __restrict format, ...);
int sprintf( char *restrict buffer, const char *restrict format, ... );
int snprintf( char *restrict buffer, size_t bufsz,
              const char *restrict format, ... );
int vfprintf( FILE *restrict stream, const char *restrict format,
              va_list vlist );
int vsnprintf(char *restrict buffer, size_t size,
              const char *restrict format, va_list vlist);
