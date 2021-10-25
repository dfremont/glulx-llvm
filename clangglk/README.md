# clangglk.h

This is a drop-in replacement for `glk.h` intended for use with the LLVM backend for Glulx.
It implements wrappers for all Glk functions so that calls to them will be translated to appropriate Glulx code (i.e. the `@glk` opcode, etc.).

One extra bit of functionality has been added: if you compile your program with `GLK_ALLOW_CSTRINGS` defined, then several Glk functions which ordinarily only accept Glulx strings (as defined in section 1.6.1 of the Glulx spec) will also accept ordinary C strings.
This is a convenience for compiling preexisting C programs which work with C strings and do not prepend the required `E0` byte that Glulx expects when calling `printf`, etc.

If a new version of Glk is released, you can use the `parseglk.py` script to generate a new version of `clangglk.h`.