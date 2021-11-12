# LLVM Backend for Glulx

This project provides an experimental LLVM backend for [Glulx](https://www.eblong.com/zarf/glulx/), allowing programs written in C or other languages with LLVM frontends to be compiled to Glulx.

Because the LLVM repository is large and has a long history, the backend itself is kept in a [separate repo](https://github.com/dfremont/llvm-project) (in the `glulx` branch) and we include it here as a submodule.
This repository contains:

* Documentation for using the backend, in this file.
* A slightly-modified version of zzo38's Glulx assembler [**glasm**](https://www.npmjs.com/package/glasm), suitable for producing `.ulx` files from the assembly code output by LLVM.
* `clangglk.h`, a drop-in replacement for `glk.h` to use with the Glulx backend. In theory, using this header allows self-contained Glk programs to be compiled directly to Glulx.
* A subset of the C standard library in the **libc** folder, providing many commonly-used functions (e.g. `printf`, `strcpy`, `time`, `realloc`) but missing some features (e.g. `fopen` does not support all file modes). This subset was sufficient for compiling Inform 6. The implementation of `printf` is from Charles Nicholson's [**nanoprintf**](https://github.com/charlesnicholson/nanoprintf).
* `libcalls.c`, which provides library functions for various essential operations not natively supported by Glulx, and which will need to be linked into most nontrivial programs.

## Installation

### Getting Started

If you just want to use the backend, you probably won't want to go through the long process of compiling it yourself, if binaries are available for your system.
I'll try to keep updated binaries for Intel Macs on the GitHub page; others are welcome to produce and distribute binaries for other platforms (with appropriate acknowledgements; see the `LICENSE`).

You can clone this repository as follows (leaving off the options about submodules if you don't need to compile the backend and only want some of the other files in the repo):

```
git clone --recurse-submodules --shallow-submodules https://github.com/dfremont/glulx-llvm
```

This should produce a folder called `glulx-llvm`.

### Compiling LLVM

Building LLVM from source takes substantial time, memory, and disk space.
On my 2019 laptop (4-core, 2.8 GHz), a full compile takes about 45 minutes and 5 GB of space (13 GB for a debug build).

See the [LLVM documentation](https://clang.llvm.org/get_started.html) for details on setting up the build, but if you aren't planning on doing development and just want the compiler, it's pretty simple if you're on a Linux or macOS system (Windows is possible too: see the previous link).
Install [CMake](https://cmake.org/download/), then try:

```
cd llvm-project
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="Glulx" ../llvm
make
```

The `bin` directory will then contain a version of _clang_ that can (only) compile to Glulx, as well as other tools -- see the next section for usage instructions.

If you're going to be working on the backend itself, I recommend installing [Ninja](https://ninja-build.org/) and using the following CMake configuration instead:

```
cmake -G "Ninja" -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="Glulx" -DLLVM_ENABLE_IDE=ON -DLLVM_OPTIMIZED_TABLEGEN=ON -DBUILD_SHARED_LIBS=ON ../llvm
```

(This will do a Debug build; the other options greatly speed up incremental builds and reduce the disk space required by many gigabytes.)

## Usage

The backend does not currently have its own assembler: it produces Glulx assembly files in the format expected by [glasm](https://www.npmjs.com/package/glasm), which you can then use to generate an `.ulx` file.
If you're compiling a single C file, for example, you can do the following:

```
clang -S -target glulx -O2 -c test.c -o test.S
node glasm/index.js < test.S > test.ulx
```

(Here I'm using the version of _glasm_ found in the eponymous folder of this repository; it has been modified slightly to accept string constants with escape sequences that LLVM assumes the assembler can recognize.)

To compile more than one file, you need to instead compile each one to a LLVM bitcode file, link those into a single file, and then compile that:

```
clang -emit-llvm -target glulx -O2 -c *.c
llvm-link -o=all.bc *.bc
llc -march=glulx -filetype=asm -O2 all.bc -o all.S
```

Note that certain primitive operations in C are not provided natively by Glulx (e.g. unsigned integer division and remainder) and so require implementation as "libcalls".
These are found in `libcalls.c`; you'll probably want to compile that file and link it into your programs to avoid getting errors from the assembler about missing symbols like `__udivsi3`.

## Limitations

The following are known limitations of the backend or other awkward aspects of compiling to Glulx:

### Unsupported LLVM Features

Although the backend is complete enough to compile substantial real C programs (e.g. Inform 6), there are a variety of LLVM features which the backend does not currently support.
Usage of these features may yield anything from a clear error message to a cryptic missing symbol when assembling; most of the time you will probably get a fatal error from the backend (e.g. `LLVM ERROR: Cannot select`) telling you to submit an LLVM bug report.
Obviously, since this is an unofficial experimental backend you should not submit a report to the main LLVM page, but you are welcome to make a GitHub Issue here (no guarantees that I will have any time to look at it, however - sorry).

Here are a few of the missing features most likely to come up:

* **Missing Libcalls.** While the backend supports all the ordinary integer and floating-point arithmetic operations, certain operations not supported natively by Glulx require library implementations.
In `libcalls.c` I provide implementations for the most common of these (in particular `__udivsi3` and `__modsi3` for unsigned integer division and remainder), but it is possible that for some code _clang_ will emit a libcall I didn't think about.
In such a case the assembler will give an error complaining about the missing symbol.
You can then look up what the libcall is supposed to do (there is likely an implementation in [compiler-rt](https://compiler-rt.llvm.org/)) and implement it yourself; I welcome pull requests adding any such missing libcalls!

* **Exception Handling.** The backend does not support the features used to implement exception handling in C++ and other languages.
If you wish you may manually implement a simple form of EH using the `__catch` and `__throw` intrinsics, which use the corresponding Glulx opcodes in the way you would expect (but see the warning under **Stack on the Heap** below).

### No Debugging Information

The backend does not currently produce debugging information, which makes debugging of compiled programs very annoying (your options: add print commands everywhere, and/or use the `__debugtrap` intrinsic to trap the Glulxe debugger and inspect raw locals).
I have not investigated what would be involved in getting the backend to produce the information needed by the Glulxe debugger; however you would also have to modify the assembler to output the addresses it picks for functions and data.

### Only Partial Standard Library

If you want to use functions provided by the C standard library, such as `strlen` or `printf`, you may find implementations in the **libc** folder.
However, I have only implemented a fragment of the whole standard library, mainly by seeing which functions were necessary to compile Inform 6.
I would welcome pull requests adding additional features or porting a serious _libc_ implementation such as [uClibc-ng](https://uclibc-ng.org/).

(If you need the C++ standard library, good luck...)

### Stack on the Heap

Since data on the Glulx stack does not have an address, the Glulx stack cannot be used as the LLVM stack.
Therefore, whenever a function requires stack objects, the backend uses `@malloc` to allocate the required memory at the start of the function and `@mfree` to release it when the function returns.
As a result:

* C programs which do not explicitly allocate memory may still require the `@malloc` opcode and therefore interpreters compliant with Glulx 3.1.0.
* If you leave the function by some unusual method (e.g. `__throw`), memory will be leaked. This includes any memory allocated using `alloca`, which ordinarily does not leak even when using `longjmp`.

Note however that simple functions which only have local variables fitting in (32-bit) Glulx locals, don't take the address of local variables, and don't use `alloca` will not require the stack at all when optimization is enabled, and so the issues above will not occur.

(Instead of using `@malloc` you could set aside a block of RAM to use as the stack, but I did not want to deal with the bookkeeping or worry about running out of space.)

## Acknowledgements

My thanks to many people who made this project possible, including:

* Andrew Plotkin, for developing Glulx and Glk, providing detailed documentation for them, and writing the invaluable Glulxe debugger.

* Chris Lattner, Vikram Adve, and many subsequent contributors, for developing LLVM.

* JF Bastien, Dan Gohman, and other contributors to the LLVM backend for WebAssembly, which I heavily copied from due to the lack of documentation on the details of writing an LLVM backend. (Thanks also to the authors of the BPF, Sparc, and x86 backends, as well as possibly others I'm forgetting).

* Andrés Amaya Garcia, for their [series](https://sourcecodeartisan.com/2020/09/13/llvm-backend-0.html) on writing an LLVM backend (it petered out incomplete at part 6, but was still helpful!).

* zzo38, for writing glasm and saving me the work of writing an assembler.

* Charles Nicholson, Wojciech Muła, and other contributors for the implementation of `printf` used in my miniature version of _libc_ ([nanoprintf](https://github.com/charlesnicholson/nanoprintf)).

* Jordan Francis Moran-Meyers, for suggesting using AppVeyor to build binaries for various platforms and showing how to do it.

* Feodor Fitsner and [AppVeyor](https://www.appveyor.com/) for generously providing the VMs used to build the binaries.

* Graham Nelson, without whose inspiring work on Inform 7 I would likely never have heard of Glulx. (And without whose inspiring work on earlier versions of Inform perhaps Glulx would not exist at all.)