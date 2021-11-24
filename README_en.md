![github ci](https://github.com/Simple-XX/SimpleKernel/workflows/CMake/badge.svg) ![last-commit](https://img.shields.io/github/last-commit/google/skia.svg) ![languages](https://img.shields.io/github/languages/count/badges/shields.svg) ![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg) [![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE) [![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_en.md) | [中文](./README.md)

# SimpleKernel

## Key Words

- kernel
- x86, x86_64, riscv64
- C, C++, AT&T ASM
- cmake
- multiboot2, opensbi

## Abstract

SimpleKernel, a simple kernel for learning. Contains the basic functionality of an operating system

Kernels with different levels of completion are available, and you can start from where you like.

Each branch:

1. boot: system startup, from the bootloader to the kernel entry
2. printf: basic character output for debud
3. parse_boot_info: reliminary parsing of the information passed by the bootloader
4. pmm: physical memory management
5. vmm: virtual memory management
6. heap: heap management
7. lib: part of c++ std/stl
8. intr: interrupt management
9. task: TODO
10. filesystem: TODO
11. device management: TODO
12. syscall: TODO
13. user mode: TODO

## Environment

- Common tools

    Cross compiler `x86_64-elf-gcc`, `riscv64-unknown-elf-gcc`, `arm-none-eabi-gcc`

    Debug `x86_64-elf-gdb`, `riscv64-unknown-elf-gdb`, `arm-none-eabi-gdb`

    Build `cmake`

    Simulator `bochs`, `qemu`

- For x86

    Bootloader `grub`
    
    ISO `xorriso`
    
- For riscv64

    Bootloader `opensbi`
    
- For arm

    TODO

## RUN

```shell
git clone https://github.com/Simple-XX/SimpleKernel.git
cd SimpleKernel/
bash ./run.sh
```

Screenshot

![](https://tva1.sinaimg.cn/large/00831rSTly1gdl6i75r60j317s0u046c.jpg)

## Directory Structure

The whole project according to the functional modules delimited molecular directory, each subdirectory and then divided header file and source file directory, so that the architecture is clear and easy to understand.

### Directory Design

- Principle

    The naming of the directory should accurately describe the basic function of the module, it is recommended to use lowercase letters and do not contain underline, dot and other special symbols;

    Directories must be placed under the parent directory that they contain, and they need to be explicitly coupled to other directories.

### Dependencies

- Principle

    New components are often dependent on existing components of the system, and the components that are directly dependent must be specified with minimal coupling.

### Header File

#### Naming

- Principle

    Header file naming should accurately describe the contents of the module contained in the file to achieve the purpose of easy to understand.

## CMake

## Test

### CI

Each push will be tested using Github Action.

## Code Style

- git commit：

    [tools/Git Commit 规范.pdf](./tools/Git Commit 规范.pdf)

- code style

    Specified by .clang-format

## TODO

- concurrent
- File system
- drive

## CONTRIBUTORS

[MRNIU](https://github.com/MRNIU)

[cy295957410](https://github.com/cy295957410)

[rakino](https://github.com/rakino)

[xiaoerlaigeid](https://github.com/xiaoerlaigeid)

[digmouse233](https://github.com/digmouse233)

[KehRoche](https://github.com/KehRoche)

## CONTRIBUTING

Free to PR!

## Thanks

[osdev](https://wiki.osdev.org)

[GRUB 在 Mac 上的安装](https://wiki.osdev.org/GRUB#Installing_GRUB_2_on_OS_X)

[multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)

《程序员的自我修养--链接、装载与库》(俞甲子 石凡 潘爱民)

[JamesM's kernel development tutorials](http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html)

[xOS](https://github.com/fengleicn/xOS)

[hurlex](http://wiki.0xffffff.org/posts/hurlex-8.html)

[howerj/os](https://github.com/howerj/os)

[cfenollosa/os-tutorial](https://github.com/cfenollosa/os-tutorial)

[omarrx024/xos](https://github.com/omarrx024/xos)

[PurpleBooth/project-title](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2%23project-title)

[0xAX weblong](http://0xax.blogspot.com/search/label/asm)

[How-to-Make-a-Computer-Operating-System](https://github.com/SamyPesse/How-to-Make-a-Computer-Operating-System)

[coding-style](https://www.kernel.org/doc/Documentation/process/coding-style.rst)

[DNKernel](https://github.com/morimolymoly/DNKernel)

[c-algorithms](https://github.com/fragglet/c-algorithms)

[Linux内核中的中断栈与内核栈的补充说明](http://blog.chinaunix.net/uid-23769728-id-3077874.html)

[Linux进程管理 (1)进程的诞生](https://www.cnblogs.com/arnoldlu/p/8466928.html)

[SynestiaOS](https://github.com/SynestiaOS/SynestiaOS)

## Donors

- [digmouse233](https://github.com/digmouse233)

- l*e

## CopyRight

MIT LICENCE
