![Build Status](https://travis-ci.org/MRNIU/SimpleKernel.svg?branch=TODO)
![last-commit](https://img.shields.io/github/last-commit/google/skia.svg)
![languages](https://img.shields.io/github/languages/count/badges/shields.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-NPL%20(The%20996%20Prohibited%20License)-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
<a href="https://996.icu"><img src="https://img.shields.io/badge/link-996.icu-red.svg"></a>
[![HitCount](http://hits.dwyl.io/MRNIU/SimpleKernel.svg)](http://hits.dwyl.io/MRNIU/SimpleKernel)

[English](https://github.com/MRNIU/SimpleKernel/blob/TODO/README_en.md) | [中文](https://github.com/MRNIU/SimpleKernel/blob/TODO/README.md)

# SimpleKernel

## Key Words

- bochs
- multiboot2
- grub2
- kernel
- Linux-like
- Operating system
- POSIX
- concurrent

## Abstract

SimpleKernel, a simple kernel for learning. Contains the basic functionality of an operating system

Kernels with different levels of completion are available, and you can start from where you like.

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

![屏幕快照 2019-03-18 上午11.16.19](https://ws2.sinaimg.cn/large/006tKfTcly1g16s32dgywj316r0u0tkt.jpg)

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
│   │   │   │   ├── boot.o
│   │   │   │   ├── boot.s
│   │   │   │   └── link.ld
│   │   │   ├── debug
│   │   │   │   ├── debug.c
│   │   │   │   └── debug.o
│   │   │   ├── intr
│   │   │   │   ├── README.md
│   │   │   │   ├── intr.c
│   │   │   │   ├── intr.h
│   │   │   │   ├── intr.o
│   │   │   │   ├── intr_s.o
│   │   │   │   └── intr_s.s
│   │   │   └── mm
│   │   │       ├── README.md
│   │   │       ├── gdt.c
│   │   │       ├── gdt.h
│   │   │       ├── gdt.o
│   │   │       ├── gdt_s.o
│   │   │       └── gdt_s.s
│   │   └── x64
│   │       └── TODO
│   ├── include
│   │   ├── README.md
│   │   ├── console.hpp
│   │   ├── cpu.hpp
│   │   ├── debug.h
│   │   ├── drv
│   │   │   ├── keyboard.h
│   │   │   └── mouse.h
│   │   ├── ds_alg
│   │   │   ├── LinkedList.c
│   │   │   ├── LinkedList.o
│   │   │   ├── alg.h
│   │   │   └── datastructure.h
│   │   ├── elf.h
│   │   ├── fs
│   │   ├── heap.c
│   │   ├── heap.h
│   │   ├── heap.o
│   │   ├── intr
│   │   │   ├── clock.c
│   │   │   ├── clock.h
│   │   │   └── clock.o
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
│   │   │   │   ├── printk.o
│   │   │   │   ├── vsprintf.c
│   │   │   │   └── vsprintf.o
│   │   │   ├── stdio.h
│   │   │   ├── stdlib.h
│   │   │   ├── string
│   │   │   │   ├── string.c
│   │   │   │   └── string.o
│   │   │   └── string.h
│   │   ├── mm
│   │   │   ├── README.md
│   │   │   ├── mm.h
│   │   │   ├── pmm.c
│   │   │   ├── pmm.h
│   │   │   ├── pmm.o
│   │   │   ├── vmm.c
│   │   │   ├── vmm.h
│   │   │   └── vmm.o
│   │   ├── multiboot2.h
│   │   ├── pic.hpp
│   │   ├── port.hpp
│   │   ├── tty.hpp
│   │   ├── vfs
│   │   │   └── vfs.h
│   │   └── vga.hpp
│   ├── kernel
│   │   ├── README.md
│   │   ├── drv
│   │   │   ├── keyboard.c
│   │   │   ├── keyboard.o
│   │   │   ├── mouse.c
│   │   │   └── mouse.o
│   │   ├── elf.c
│   │   ├── elf.o
│   │   ├── fs.c
│   │   ├── fs.o
│   │   ├── kernel.c
│   │   ├── kernel.o
│   │   ├── multiboot2.c
│   │   ├── multiboot2.o
│   │   ├── vfs.c
│   │   └── vfs.o
│   ├── kernel.kernel
│   └── test
│       ├── test.c
│       ├── test.h
│       └── test.o
└── tools
    ├── bochs.sh
    ├── i386-elf-binutils.sh
    ├── i386-elf-gcc.sh
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
- concurrent
- File system
- drive
- Virtual memory management

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