POSIX-UEFI
==========

<blockquote>We hate that horrible and ugly UEFI API, we want the usual POSIX!</blockquote>

------------------------------------------------------------------------------------------------------------------------

NOTE: a smartypants on reddit is worried that I'm supposedly "hiding something" because of the use of `-Wno-builtin-declaration-mismatch`
(gcc) and `-Wno-incompatible-library-redeclaration` (Clang) flags. **Here's my answer to you: I hide nothing**, that flag is only
needed because you can disable transparent UTF-8 conversion. You see, under the hood UEFI uses 16 bit characters, and for example
`strlen(wchar_t *str)` or `main(int argc, wchar_t *argv)` isn't exactly POSIX-standard, that's why there's a need for that flag.
You should know that had you have spent more time learning or just *reading this README* instead of falsely accusing others on reddit.

------------------------------------------------------------------------------------------------------------------------

This is a very small build environment that helps you to develop for UEFI under Linux (and other POSIX systems). It was
greatly inspired by [gnu-efi](https://sourceforge.net/projects/gnu-efi) (big big kudos to those guys), but it is a lot
smaller, easier to integrate (works with LLVM Clang and GNU gcc both) and easier to use because it provides a POSIX like
API for your UEFI application.

An UEFI environment consist of two parts: a firmware with GUID protocol interfaces and a user library. We cannot change
the former, but we can make the second friendlier. That's what POSIX-UEFI does for your application. It is a small API
wrapper library around the GUID protocols, not a fully blown POSIX compatible libc implementation.

You have two options on how to integrate it into your project:

Distributing as Static Library
------------------------------

Same method as with gnu-efi, not really recommended. In the `uefi` directory, run
```sh
$ USE_GCC=1 make
```
This will create `build/uefi` with all the necessary files in it. These are:

 - **crt0.o**, the run-time that bootstraps POSIX-UEFI
 - **link.ld**, the linker script you must use with POSIX-UEFI (same as with gnu-efi)
 - **libuefi.a**, the library itself
 - **uefi.h**, the all-in-one C / C++ header

You can use this and link your application with it, but you won't be able to recompile it, plus you're on your own with
the linking and converting.

Strictly speaking you'll only need **crt0.o** and **link.ld**, that will get you started and will call your application's
"main()", but to get libc functions like memcmp, strcpy, malloc or fopen, you'll have to link with **libuefi.a** too.

For now this only works with gcc, because Clang is configured in a way to directly create PE files, so it cannot create
nor link with static ELF .a files.

Distributing as Source
----------------------

This is the preferred way, as it also provides a Makefile to set up your toolchain properly.

 1. simply copy the `uefi` directory into your source tree (or set up a git submodule and a symlink). A dozen files, about 132K in total.
 2. create an extremely simple **Makefile** like the one below
 3. compile your code for UEFI by running `make`

```
TARGET = helloworld.efi
include uefi/Makefile
```
An example **helloworld.c** goes like this:
```c
#include <uefi.h>

int main(int argc, char **argv)
{
    printf("Hello World!\n");
    return 0;
}
```
By default it uses Clang + lld, and PE is generated directly without conversion. If `USE_GCC` is set, then the host native's
GNU gcc + ld is used to create a shared object and get converted into an .efi file with objcopy, just like how gnu-efi does.

**NOTE**: if you don't want to clone this entire repo, just the `uefi` directory,
```
git clone --no-checkout https://gitlab.com/bztsrc/posix-uefi.git .
git sparse-checkout set --no-cone '/uefi/*'
git checkout
```

### Available Makefile Options

| Variable   | Description                                                                                                       |
|------------|-------------------------------------------------------------------------------------------------------------------|
| `TARGET`   | the target application (required)                                                                                 |
| `SRCS`     | list of source files you want to compile (defaults to \*.c \*.S)                                                  |
| `CFLAGS`   | compiler flags you want to use (empty by default, like "-Wall -pedantic -std=c99")                                |
| `LDFLAGS`  | linker flags you want to use (I don't think you'll ever need this, just in case)                                  |
| `LIBS`     | additional libraries you want to link with (like "-lm", only static .a libraries allowed)                         |
| `EXTRA`    | any additional object files you might want to link with, these are also called as makefile rules before compiling |
| `ALSO`     | additional makefile rules to be called after compiling                                                            |
| `OUTDIR`   | if given, then your project's object files are generated into this directory (by default not set)                 |
| `USE_GCC`  | set this if you want native GNU gcc + ld + objcopy instead of LLVM Clang + Lld                                    |
| `ARCH`     | the target architecture                                                                                           |

Here's a more advanced **Makefile** example:
```
ARCH = x86_64
TARGET = helloworld.efi
SRCS = $(wildcard *.c)
CFLAGS = -pedantic -Wall -Wextra -Werror --std=c11 -O2
LDFLAGS =
LIBS = -lm
OUTDIR = build/loader

USE_GCC = 1
include uefi/Makefile
```
The build environment configurator was created in a way that it can handle any number of architectures, however only
`x86_64` crt0 has been throughfully tested for now. There's an `aarch64` and a `riscv64` crt0 too, but since I don't
have neither an ARM UEFI, nor a RISC-V UEFI board, these **haven't been tested at all**. Should work though. If you want
to port it to another architecture, all you need is a setjmp struct in uefi.h, and an appropriate crt_X.c file. That's
it. Everything else was coded in an architecture independent way.

### Available Configuration Options

These can be set at the beginning of `uefi.h`.

| Define                | Description                                                                               |
|-----------------------|-------------------------------------------------------------------------------------------|
| `UEFI_NO_UTF8`        | Do not use transparent UTF-8 conversion between the application and the UEFI interface    |
| `UEFI_NO_TRACK_ALLOC` | Do not keep track of allocated buffers (faster, but causes out of bound reads on realloc) |

Notable Differences to POSIX libc
---------------------------------

This library is nowhere near as complete as glibc or musl for example. It only provides the very basic libc functions
for you, because simplicity was one of its main goals. It is the best to say this is just wrapper around the UEFI API,
rather than a POSIX compatible libc.

All strings in the UEFI environment are stored with 16 bits wide characters. The library provides `wchar_t` type for that,
and the `UEFI_NO_UTF8` define to convert between `char` and `wchar_t` transparently. If you have `UEFI_NO_UTF8`, then for
example your main() will NOT be like `main(int argc, char **argv)`, but `main(int argc, wchar_t **argv)` instead. All
the other string related libc functions (like strlen() for example) will use this wide character type too. For this reason,
you must specify your string literals with `L""` and characters with `L''`. To handle both configurations, `char_t` type is
defined, which is either `char` or `wchar_t`, and the `CL()` macro which might add the `L` prefix to constant literals.
Functions that are supposed to handle characters in int type (like `getchar`, `putchar`), do not truncate to unsigned char,
rather to wchar_t.

Sadly UEFI has no concept of reallocation. AllocatePool does not accept input, and there's no way to query the size of an
already allocated buffer. So we are left with two bad options with `realloc`:
1. we keep track of sizes ourselves, which means more complexcity and a considerable overhead, so performance loss.
2. make peace with the fact that copying data to the new buffer unavoidably reads out of bounds from the old buffer.
You can choose this latter with the `UEFI_NO_TRACK_ALLOC` define.

File types in dirent are limited to directories and files only (DT_DIR, DT_REG), but for stat in addition to S_IFDIR and
S_IFREG, S_IFIFO (for console streams: stdin, stdout, stderr), S_IFBLK (for Block IO) and S_IFCHR (for Serial IO) also
returned.

Note that `getenv` and `setenv` aren't POSIX standard, because UEFI environment variables are binary blobs.

That's about it, everything else is the same.

List of Provided POSIX Functions
--------------------------------

### dirent.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| opendir       | as usual, but might accept wide char strings                               |
| readdir       | as usual                                                                   |
| rewinddir     | as usual                                                                   |
| closedir      | as usual                                                                   |

Because UEFI has no concept of device files nor of symlinks, dirent fields are limited and only DT_DIR and DT_REG supported.

### stdlib.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| atoi          | as usual, but might accept wide char strings and understands "0x" prefix   |
| atol          | as usual, but might accept wide char strings and understands "0x" prefix   |
| strtol        | as usual, but might accept wide char strings                               |
| malloc        | as usual                                                                   |
| calloc        | as usual                                                                   |
| realloc       | as usual                                                                   |
| free          | as usual                                                                   |
| abort         | as usual                                                                   |
| exit          | as usual                                                                   |
| exit_bs       | leave this entire UEFI bullshit behind (exit Boot Services)                |
| mbtowc        | as usual (UTF-8 char to wchar_t)                                           |
| wctomb        | as usual (wchar_t to UTF-8 char)                                           |
| mbstowcs      | as usual (UTF-8 string to wchar_t string)                                  |
| wcstombs      | as usual (wchar_t string to UTF-8 string)                                  |
| srand         | as usual                                                                   |
| rand          | as usual, but uses EFI_RNG_PROTOCOL if possible                            |
| getenv        | pretty UEFI specific                                                       |
| setenv        | pretty UEFI specific                                                       |

```c
int exit_bs();
```
Exit Boot Services. Returns 0 on success. You won't be able to return from main() after calling this successfully, you must
transfer control directly.

```c
uint8_t *getenv(char_t *name, uintn_t *len);
```
Query the value of environment variable `name`. On success, `len` is set, and a malloc'd buffer returned. It is
the caller's responsibility to free the buffer later. On error returns NULL.
```c
int setenv(char_t *name, uintn_t len, uint8_t *data);
```
Sets an environment variable by `name` with `data` of length `len`. On success returns 1, otherwise 0 on error.

### stdio.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| remove        | as usual, but might accept wide char strings                               |
| fopen         | as usual, but might accept wide char strings, also for mode                |
| fclose        | as usual                                                                   |
| fflush        | as usual                                                                   |
| fread         | as usual, only real files and blk io accepted (no stdin)                   |
| fwrite        | as usual, only real files and blk io accepted (no stdout nor stderr)       |
| fseek         | as usual, only real files and blk io accepted (no stdin, stdout, stderr)   |
| ftell         | as usual, only real files and blk io accepted (no stdin, stdout, stderr)   |
| feof          | as usual, only real files and blk io accepted (no stdin, stdout, stderr)   |
| fprintf       | as usual, might be wide char strings, BUFSIZ, files, ser, stdout, stderr   |
| printf        | as usual, might be wide char strings, max BUFSIZ, stdout only              |
| sprintf       | as usual, might be wide char strings, max BUFSIZ                           |
| vfprintf      | as usual, might be wide char strings, BUFSIZ, files, ser, stdout, stderr   |
| vprintf       | as usual, might be wide char strings, max BUFSIZ, stdout only              |
| vsprintf      | as usual, might be wide char strings, max BUFSIZ                           |
| snprintf      | as usual, might be wide char strings                                       |
| vsnprintf     | as usual, might be wide char strings                                       |
| getchar       | as usual, blocking, stdin only (no stream redirects), returns UNICODE      |
| getchar_ifany | non-blocking, returns 0 if there was no key press, UNICODE otherwise       |
| putchar       | as usual, stdout only (no stream redirects)                                |

String formating is limited; only supports padding via positive number prefixes, `%d`, `%i`, `%x`, `%X`, `%c`, `%s`, `%q` and
`%p` (no `%e`, `%f`, `%g`, no asterisk and dollar). When `UEFI_NO_UTF8` is defined, then formating operates on wchar_t, so
it also supports the non-standard `%S` (printing an UTF-8 string) and `%Q` (printing an escaped UTF-8 string). These
functions don't allocate memory, but in return the total length of the output string cannot be longer than `BUFSIZ`
(8k if you haven't defined otherwise), except for the variants which have a maxlen argument. For convenience, `%D` requires
`efi_physical_address_t` as argument, and it dumps memory, 16 bytes or one line at once. With the padding modifier you can
dump more lines, for example `%5D` gives you 5 lines (80 dumped bytes).

File open modes: `"r"` read, `"w"` write, `"a"` append. Because of UEFI peculiarities, `"wd"` creates directory.

Special "device files" you can open:

| Name                | Description                                                          |
|---------------------|----------------------------------------------------------------------|
| `/dev/stdin`        | returns ST->ConIn                                                    |
| `/dev/stdout`       | returns ST->ConOut, fprintf                                          |
| `/dev/stderr`       | returns ST->StdErr, fprintf                                          |
| `/dev/serial(baud)` | returns Serial IO protocol, fread, fwrite, fprintf                   |
| `/dev/disk(n)`      | returns Block IO protocol, fseek, ftell, fread, fwrite, feof         |

With Block IO, fseek and buffer size for fread and fwrite is always truncated to the media's block size. So fseek(513)
for example will seek to 512 with standard block sizes, and 0 with large 4096 block sizes. To detect the media's block
size, use fstat.
```c
if(!fstat(f, &st))
    block_size = st.st_size / st.st_blocks;
```
To interpret a GPT, there are typedefs like `efi_partition_table_header_t` and `efi_partition_entry_t` which you can point
to the read data.

### string.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| memcpy        | as usual, works on bytes                                                   |
| memmove       | as usual, works on bytes                                                   |
| memset        | as usual, works on bytes                                                   |
| memcmp        | as usual, works on bytes                                                   |
| memchr        | as usual, works on bytes                                                   |
| memrchr       | as usual, works on bytes                                                   |
| memmem        | as usual, works on bytes                                                   |
| memrmem       | as usual, works on bytes                                                   |
| strcpy        | might work on wide char strings                                            |
| strncpy       | might work on wide char strings                                            |
| strcat        | might work on wide char strings                                            |
| strncat       | might work on wide char strings                                            |
| strcmp        | might work on wide char strings                                            |
| strncmp       | might work on wide char strings                                            |
| strdup        | might work on wide char strings                                            |
| strchr        | might work on wide char strings                                            |
| strrchr       | might work on wide char strings                                            |
| strstr        | might work on wide char strings                                            |
| strtok        | might work on wide char strings                                            |
| strtok_r      | might work on wide char strings                                            |
| strlen        | might work on wide char strings                                            |

### sys/stat.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| stat          | as usual, but might accept wide char strings                               |
| fstat         | UEFI doesn't have fd, so it uses FILE\*                                    |
| mkdir         | as usual, but might accept wide char strings, and mode unused              |

Because UEFI has no concept of device major and minor number nor of inodes, struct stat's fields are limited.
The actual implementation of `fstat` is in stdio.c, because it needs to access static variables defined there.

### time.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| localtime     | argument unused, always returns current time in struct tm                  |
| mktime        | as usual                                                                   |
| time          | as usual                                                                   |

### unistd.h

| Function      | Description                                                                |
|---------------|----------------------------------------------------------------------------|
| usleep        | as usual (uses BS->Stall)                                                  |
| sleep         | as usual                                                                   |
| unlink        | as usual, but might accept wide char strings                               |
| rmdir         | as usual, but might accept wide char strings                               |

Accessing UEFI Services
-----------------------

It is very likely that you want to call UEFI specific functions directly. For that, POSIX-UEFI specifies some globals
in `uefi.h`:

| Global Variable | Description                                              |
|-----------------|----------------------------------------------------------|
| `*BS`, `gBS`    | *efi_boot_services_t*, pointer to the Boot Time Services |
| `*RT`, `gRT`    | *efi_runtime_t*, pointer to the Runtime Services         |
| `*ST`, `gST`    | *efi_system_table_t*, pointer to the UEFI System Table   |
| `IM`            | *efi_handle_t* of your Loaded Image                      |

The EFI structures, enums, typedefs and defines are all converted to ANSI C standard POSIX style, for example
BOOLEAN ->  boolean_t, UINTN -> uintn_t, EFI_MEMORY_DESCRIPTOR -> efi_memory_descriptor_t, and of course
EFI_BOOT_SERVICES -> efi_boot_services_t.

Calling UEFI functions is as simple as with EDK II, just do the call, no need for "uefi_call_wrapper":
```c
    ST->ConOut->OutputString(ST->ConOut, L"Hello World!\r\n");
```
(Note: unlike with printf, with OutputString you must use `L""` and also print carrige return `L"\r"` before `L"\n"`. These
are the small things that POSIX-UEFI does for you to make your life comfortable.)

There are two additional, non-POSIX calls in the library. One is `exit_bs()` to exit Boot Services, and the other is
a non-blocking version `getchar_ifany()`.

Unlike gnu-efi, POSIX-UEFI does not pollute your application's namespace with unused GUID variables. It only provides
header definitions, so you must create each GUID instance if and when you need them.

Example:
```c
efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
efi_gop_t *gop = NULL;

status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
```

Also unlike gnu-efi, POSIX-UEFI does not provide standard EFI headers. It expects that you have installed those under
/usr/include/efi from EDK II or gnu-efi, and POSIX-UEFI makes it possible to use those system wide headers without
naming conflicts. POSIX-UEFI itself ships the very minimum set of typedefs and structs (with POSIX-ized names).
```c
#include <efi.h>
#include <uefi.h> /* this will work as expected! Both POSIX-UEFI and EDK II / gnu-efi typedefs available */
```
The advantage of this is that you can use the simplicity of the POSIX-UEFI library and build environment, while getting
access to the most up-to-date protocol and interface definitions at the same time.

__IMPORTANT NOTE__

In some cases the combination of GNU-EFI headers and Clang might incorrectly define `uint64_t` as 32 bits. If this happens,
then
```c
#undef __STDC_VERSION__
#include <efi.h>
#include <uefi.h>
```
should workaround the problem by avoiding the inclusion of stdint.h and defining uint64_t by efibind.h as `unsigned long long`.

License
-------

POSIX_UEFI is licensed under the terms of the MIT license.

Contributors
------------

I'd like to say thanks to @vladimir132 for a through testing and very accurate and detailed feedbacks.

Cheers,

bzt
