# Mini C Standard Library

This is an implementation of a fragment of the C Standard Library for use when compiling C programs to Glulx.
The supported features are summarized here; see the code for details.

### assert.h

* the `assert` macro

### ctype.h

* `isalnum`, `isdigit`, `isspace`, etc.
* `tolower`, `toupper`

### errno.h

* the `errno` global variable
* various error codes (`EDOM`, `ERANGE`, `EINVAL`, etc.)

### math.h

* `INFINITY`, `NAN` macros
* `isinf`, `isfinite`, `isnan` functions
* `ldexp`, `frexp`
* `sqrt`, `exp`, `log`, `pow`
* trigonometric functions
* `ceil`, `floor`

**Note:** these functions are compliant with the C99 standard.
In particular, operations like `pow` and `sin` not only perform the calculation provided by the corresponding Glulx opcode, but also set `errno` for invalid argument values.
If you don't care about `errno` and are happy to let NaNs propagate through your program (or are confident you won't get any), you can directly invoke the Glulx opcodes through intrinsics like `__sin`, `__pow`, etc.

### stdarg.h

* `va_list` type for variable-argument lists
* `va_start`, `va_end`, and `va_arg` macros for variadic functions

### stdint.h

* `int8_t`, `int16_t`, `int32_t` and assorted other types

### stdio.h

* `stdout` and `stderr`, mapped to the current Glk stream
* `fopen`, `fclose`
* `fflush` (does nothing; unnecessary in Glk)
* `feof`, `ferror`
* `fseek`, `fsetpos`, `fgetpos`
* `fread`, `fwrite`, `fgets`, `fputs`, `fgetc`, `fputc`
* `remove`
* `putchar`, `puts`
* `printf`, `fprintf`, `sprintf`, `vsnprintf`, etc.

**Note:** Some functionality may seem surprising if you're not familiar with Glk; e.g. `fopen` will likely munge the filename you intended and stick on a `.glkdata` extension, if the underlying Glk implementation in your interpreter decides to (this is unavoidable).

### stdlib.h

* `exit`
* `malloc`, `calloc`, `realloc`, `free`
* `qsort`

### string.h

* `strlen`, `strcmp`, `strncmp`, `strchr` (also the non-standard `strnlen`)
* `strcpy`, `strncpy`, `strcat`
* `atoi`, `strtol`
* `memcmp`, `memcpy`, `memmove`, `memset`

### time.h

* `time`, `localtime`
* `strftime` (extremely few format strings supported at the moment)