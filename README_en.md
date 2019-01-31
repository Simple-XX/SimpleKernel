![Build Status](https://travis-ci.org/MRNIU/SimpleKernel.svg?branch=TODO)
![last-commit](https://img.shields.io/github/last-commit/google/skia.svg)
![languages](https://img.shields.io/github/languages/count/badges/shields.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

[English](https://github.com/MRNIU/SimpleKernel/blob/TODO/README_en.md) | [中文](https://github.com/MRNIU/SimpleKernel/blob/TODO/README.md)
# SimpleKernel

## Abstract

SimpleKernel, a simple kernel for learning. Contains the basic functionality of an operating system

Kernels with different levels of completion are available, and you can start from where you like.

Languages: 

- x86 Assembly
- C
- C++
- Shell
- make

## Environment

- Ubuntu Linux

    - Bochs
        Install by compiling
    - i386-elf Cross-compiling tool chain
        Install by compiling (./tools/)

- Mac
    - Bochs

            brew install bochs

    - i386-elf Cross-compiling tool chain

        ```shell
        brew install i386-elf-binutils
        brew install i386-elf-gcc
        cd SimpleKernel/ && brew install ./i386-elf-grub.rb
        ```

NOTE：
Mac 10.14.1, bochs 2.6.9, i386-elf-binutils 2.31.1, i386-elf-gcc 8.2.0 pass-test.

## RUN

    git clone https://github.com/MRNIU/SimpleKernel.git
    cd SimpleKernel/
    sh setup.sh

Input `c` to bochs to run.

Screenshot

![屏幕快照 2018-12-10 下午8.40.12](https://lh3.googleusercontent.com/-bMiOQG70raM/XA5ijMqVcLI/AAAAAAAAAGU/YB_HvVl8JD4PiKbV1UHfQA4HqeD6xWCxACHMYCw/I/%255BUNSET%255D)

![屏幕快照 2018-12-10 下午8.40.38](https://lh3.googleusercontent.com/-Lp2Xea_cLgM/XA5ijHG93RI/AAAAAAAAAGc/wpYAqRo7wGIABAAi6jGz3NRhASB2aOQywCHMYCw/I/%255BUNSET%255D)

![屏幕快照 2018-12-10 下午8.40.50](https://lh3.googleusercontent.com/-DwLqMLIWGps/XA5ijD1nZvI/AAAAAAAAAGY/PeRQd2FN8qoOEGq4LEx1vxgmFCmq8qUYACHMYCw/I/%255BUNSET%255D)

If you need more bochs usage, please refer to bochs official documentation.


## QUICK START

```
├── LICENSE
├── README.md
├── README_en.md
├── bochsrc.txt
├── debug_info
│   ├── bochsout.txt
│   ├── diff.log
│   ├── error.log
│   └── normal.log
├── docs
│   └── README.md
├── fs.img
├── iso
│   └── boot
│       ├── grub
│       │   └── grub.cfg
│       └── kernel.kernel
├── related_docs
├── setup.sh
├── simplekernel.img
├── simplekernel.iso
├── src
│   ├── Makefile
│   ├── READMD.md
│   ├── arch
│   │   ├── README.md
│   │   ├── i386
│   │   │   ├── README.md
│   │   │   ├── boot
│   │   │   │   ├── boot.s
│   │   │   │   └── link.ld
│   │   │   ├── debug
│   │   │   │   └── debug.c
│   │   │   ├── intr
│   │   │   │   ├── README.md
│   │   │   │   ├── intr.c
│   │   │   │   ├── intr.h
│   │   │   │   └── intr_s.s
│   │   │   └── mm
│   │   │       ├── README.md
│   │   │       ├── gdt.c
│   │   │       ├── gdt.h
│   │   │       └── gdt_s.s
│   │   └── x64
│   │       └── TODO
│   ├── include
│   │   ├── DataStructure
│   │   │   ├── DataStructuer.h
│   │   │   ├── LinkedList.c
│   │   │   ├── LinkedList.cpp
│   │   │   ├── Queue.cpp
│   │   │   ├── SortAlgorithm.cpp
│   │   │   └── Stack.cpp
│   │   ├── README.md
│   │   ├── console.hpp
│   │   ├── cpu.hpp
│   │   ├── debug.h
│   │   ├── drv
│   │   │   ├── keyboard.h
│   │   │   └── mouse.h
│   │   ├── elf.h
│   │   ├── intr
│   │   │   ├── clock.c
│   │   │   └── clock.h
│   │   ├── kernel.h
│   │   ├── libc
│   │   │   ├── README.md
│   │   │   ├── assert.h
│   │   │   ├── stdarg.h
│   │   │   ├── stdbool.h
│   │   │   ├── stddef.h
│   │   │   ├── stdint.h
│   │   │   ├── stdio
│   │   │   │   ├── printk.c
│   │   │   │   └── vsprintf.c
│   │   │   ├── stdio.h
│   │   │   ├── string
│   │   │   │   └── string.c
│   │   │   └── string.h
│   │   ├── mm
│   │   │   ├── README.md
│   │   │   ├── mm.h
│   │   │   ├── pmm.c
│   │   │   └── pmm.h
│   │   ├── multiboot2.h
│   │   ├── pic.hpp
│   │   ├── port.hpp
│   │   ├── tty.hpp
│   │   └── vga.hpp
│   └── kernel
│       ├── README.md
│       ├── drv
│       │   ├── keyboard.c
│       │   └── mouse.c
│       ├── elf.c
│       ├── kernel.c
│       └── multiboot2.c
└── tools
    ├── bochs.sh
    ├── i386-elf-binutils.sh
    ├── i386-elf-gcc.sh
    ├── i386-elf-grub.rb
    └── i386-elf-grub.sh
```

## Test

Not yet.

### CI

See `.travis.yml`

## Code Style

## Image Files

simplekernel.img: multiboot1 boot(not updated)

simplekernel.iso: multiboot2 boot

fs.img: file system, unuse

## TODO

- fix bugs
- mouse input
- memory management
- checkstyle

## CONTRIBUTORS

[MRNIU](https://github.com/MRNIU)

## CONTRIBUTING

Read CONTRIBUTING.md.

## Thanks

[osdev](https://wiki.osdev.org)

[install GRUB on Mac](https://wiki.osdev.org/GRUB#Installing_GRUB_2_on_OS_X)

[multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)

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

## CopyRight

MIT LICENCE