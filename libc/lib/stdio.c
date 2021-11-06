
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <clangglk.h>

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 1
#define NANOPRINTF_VISIBILITY_STATIC
#include "nanoprintf.h"
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

// Internals

static FILE stdout_file = { .is_glk=1, .stream=0, .maxsize=0 };

static void _open_buffer_file(FILE *file, char *buf, size_t maxsize) {
	file->is_glk = 0;
	file->buffer = buf;
	file->maxsize = maxsize;
	file->pos = 0;
	file->error = 0;
}

static FILE *_open_glk_file(strid_t stream) {
	FILE *file = malloc(sizeof(FILE));
	file->is_glk = 1;
	file->stream = stream;
	file->maxsize = SIZE_MAX;
	file->pos = 0;
	file->error = 0;
	return file;
}

static void _close_glk_file(FILE *file) {
	free(file);
}

// Public interface

FILE *stdin;
FILE *stdout = &stdout_file;
FILE *stderr = &stdout_file;

FILE *fopen(const char *restrict filename, const char *restrict mode) {
	unsigned glkmode;
	if (mode[0] == 'r')
		glkmode = filemode_Read;
	else if (mode[0] == 'w')
		glkmode = filemode_Write;
	else if (mode[0] == 'a')
		glkmode = filemode_WriteAppend;
	else {
		errno = EINVAL;
		return NULL;
	}
	unsigned usage = fileusage_Data;
	unsigned extendedIndex;
	if (mode[1] == 'b') {
		usage |= fileusage_BinaryMode;
		extendedIndex = 2;
	} else {
		usage |= fileusage_TextMode;
		extendedIndex = 1;
	}
	unsigned clearFile = 0;
	if (mode[extendedIndex] == '+') {
		if (glkmode == filemode_WriteAppend)
			return NULL;		// unsupported mode
		else if (glkmode == filemode_Write)
			clearFile = 1;
		glkmode = filemode_ReadWrite;
	}

	frefid_t fileref = glk_fileref_create_by_name(usage, (char *) filename, 0);
	if (!fileref) {
		errno = ENOENT;
		return NULL;
	}

	if (glk_fileref_does_file_exist(fileref)) {
		if (clearFile)
			glk_fileref_delete_file(fileref);
	} else if (glkmode == filemode_Read) {
		errno = ENOENT;
		return NULL;
	}

	strid_t stream = glk_stream_open_file(fileref, glkmode, 0);
	if (!stream) {
		errno = ENOENT;
		return NULL;
	}

	glk_fileref_destroy(fileref);

	return _open_glk_file(stream);
}

int fclose(FILE *file) {
	if (file->is_glk) {
		glk_stream_close(file->stream, NULL);
		_close_glk_file(file);
	}
	return 0;
}

int fflush(FILE *file) {
	return 0;
}

int feof(FILE *file) {
	return (file->pos >= file->maxsize);
}

int ferror(FILE *file) {
	return file->error;
}

int fseek(FILE *file, long offset, int origin) {
	if (file->is_glk) {
		glk_stream_set_position(file->stream, offset, origin);
	} else {
		size_t new_pos;
		switch (origin) {
			case SEEK_SET: new_pos = offset; break;
			case SEEK_CUR: new_pos = file->pos + offset; break;
			case SEEK_END:
				if (offset > 0 || file->maxsize == SIZE_MAX)
					return 1;
				new_pos = file->maxsize + offset;
				break;
			default:
				errno = EINVAL;
				return 1;
		}
		if (new_pos < 0 || new_pos >= file->maxsize)
			return 1;
		file->pos = new_pos;
	}
	return 0;
}

int fsetpos(FILE *file, const fpos_t *pos) {
	if (file->is_glk)
		glk_stream_set_position(file->stream, pos->index, seekmode_Start);
	else
		file->pos = pos->index;
	return 0;
}

int fgetpos(FILE *restrict file, fpos_t *restrict pos) {
	if (file->is_glk)
		pos->index = glk_stream_get_position(file->stream);
	else
		pos->index = file->pos;
	return 0;
}

size_t fread(void *restrict buffer, size_t size, size_t count,
             FILE *restrict file ) {
	unsigned char *buf = buffer;
	size_t num;
	for (num = 0; num < count; ++num) {
		for (size_t i = size; i; --i) {
			int ch = fgetc(file);
			if (ch == EOF)
				goto done;
			*buf++ = ch;
		}
	}
	done:
	return num;
}

size_t fwrite(const void *restrict buffer, size_t size, size_t count,
              FILE *restrict file ) {
	const unsigned char *buf = buffer;
	size_t num;
	for (num = 0; num < count; ++num) {
		for (size_t i = size; i; --i) {
			if (fputc(*buf++, file) == EOF)
				goto done;
		}
	}
	done:
	return num;
}

char *fgets(char *restrict str, int count, FILE *restrict file) {
	char *s = str;
	while (--count) {
		int ch = fgetc(file);
		if (ch == EOF)
			break;
		*s++ = ch;
		if (ch == '\n')
			break;
	}
	if (s == str)
		return NULL;
	*s = 0;
	return str;
}

int fputs(const char *restrict str, FILE *restrict file) {
	for (; *str; ++str) {
		if (fputc(*str, file) == EOF)
			return EOF;
	}
	return 1;
}

int fgetc(FILE *file) {
	if (file->pos >= file->maxsize)
		return EOF;
	if (file->is_glk) {
		int ch = glk_get_char_stream(file->stream);
		if (ch == -1) {
			file->pos = SIZE_MAX;
			return EOF;
		}
		return ch;
	} else {
		assert(file->buffer && "fgetc called on buffer-file with NULL buffer");
		return (unsigned char) file->buffer[file->pos++];
	}
}

int fputc(int ch, FILE *file) {
	unsigned char cch = ch;
	if (file->is_glk) {
		if (file == stdout)
			glk_put_char(cch);
		else
			glk_put_char_stream(file->stream, cch);
	} else {
		if (file->buffer) {
			if (file->pos >= file->maxsize) {
				file->error = 1;
				return EOF;
			}
			file->buffer[file->pos++] = cch;
		} else {
			// this is a NULL buffer used only to count the number of characters
			// that would ordinarily be printed (see e.g. vsnprintf)
			file->pos++;
		}
	}
	return cch;
}

int remove(const char *filename) {
	unsigned usage = fileusage_Data | fileusage_BinaryMode;
	frefid_t fileref = glk_fileref_create_by_name(usage, (char *) filename, 0);
	if (!fileref || !glk_fileref_does_file_exist(fileref))
		return 1;
	glk_fileref_delete_file(fileref);
	glk_fileref_destroy(fileref);
	return 0;
}

int putchar(int ch) {
	return fputc(ch, stdout);
}

int puts(const char *str) {
	if (fputs(str, stdout) == EOF)
		return EOF;
	return fputc('\n', stdout);
}

int printf(const char *restrict format, ...) {
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfprintf(stdout, format, arg);
	va_end(arg);

	return rv;
}

int fprintf(FILE *restrict stream, const char *restrict format, ...) {
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfprintf(stream, format, arg);
	va_end(arg);

	return rv;
}

int sprintf(char *restrict buffer, const char *restrict format, ...) {
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vsnprintf(buffer, SIZE_MAX, format, arg);
	va_end(arg);

	return rv;
}

int snprintf(char *restrict buffer, size_t size,
             const char *restrict format, ...) {
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vsnprintf(buffer, size, format, arg);
	va_end(arg);

	return rv;
}

int vsnprintf(char *restrict buffer, size_t size,
              const char *restrict format, va_list vlist) {
	if (size == 0) {		// character-counting mode
		size = SIZE_MAX;
		buffer = NULL;
	} else {
		assert(buffer && "vsnprintf given NULL buffer");
		size = size - 1;		// save last byte for null terminator
	}

	FILE file;
	_open_buffer_file(&file, buffer, size);

	int rv = vfprintf(&file, format, vlist);
	if (buffer && rv >= 0)
		buffer[rv > size ? size : rv] = 0;		// null terminator

	return rv;
}

static void my_npf_putc(int c, void *ctx) {
	fputc(c, ctx);
}

int vfprintf(FILE *restrict file, const char *restrict format, va_list vlist) {
	return npf_vpprintf(my_npf_putc, file, format, vlist);
}
