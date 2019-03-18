![Build Status](https://travis-ci.org/MRNIU/SimpleKernel.svg?branch=TODO)
![last-commit](https://img.shields.io/github/last-commit/google/skia.svg)
![languages](https://img.shields.io/github/languages/count/badges/shields.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

[English](https://github.com/MRNIU/SimpleKernel/blob/TODO/README_en.md) | [中文](https://github.com/MRNIU/SimpleKernel/blob/TODO/README.md)
# SimpleKernel

## 关键词

- Grub2
- UXIN-like
- 多任务
- ELF
- IDE disk
- EXT2 文件系统
- POSIX API
- libc
- 一些 shell命令

## 简介

SimpleKernel，一个用来练手的简单内核。提供了各个阶段完成度不同的内核，你可以从自己喜欢的地方开始。

使用的语言：

- x86 汇编

- C

- C++

- Shell

- make

- x86


## 开发环境

- Ubuntu Linux

    - Bochs
        编译安装

    - i386-elf 交叉编译工具链
        编译安装（./tools/）

- Mac
    - Bochs

            brew install bochs

    - i386-elf 交叉编译工具链

            brew install i386-elf-binutils
            brew install i386-elf-gcc
            cd SimpleKernel/ && brew install tools/i386-elf-grub.rb

注：
Mac 10.14.1，bochs 2.6.9，i386-elf-binutils 2.31.1，i386-elf-gcc 8.2.0 测试通过。

## 如何运行

    git clone https://github.com/MRNIU/SimpleKernel.git
    cd SimpleKernel/
    sh setup.sh

在出现的 bochs 命令行中 输入 `c` 即可运行。

运行截图

![屏幕快照 2019-03-18 上午11.16.19](https://ws3.sinaimg.cn/large/006tKfTcly1g16s317uw8j316r0u0tkt.jpg)

更多 bochs 使用方式请参考 bochs 的相关资料。


## 快速上手

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

## 测试

目前只有很简陋的 CI。

### 自动化集成

合并到 master 分支时需要通过 Travis CI 测试。编译没有错误即可。

## 代码风格

## 镜像文件

simplekernel.img 是 1.44 软盘，我们的内核就在这里。

## TODO

- 修复 bug
- 鼠标输入处理
- debug 函数
- 添加编码规范测试
- 并发
- 文件系统
- 设备驱动
- 虚拟内存管理

## 贡献者

[MRNIU](https://github.com/MRNIU)

您也可以在贡献者名单中参看所有参与该项目的开发者。

## 贡献

请阅读 CONTRIBUTING.md。

## 感谢

此项目参考了很多优秀的项目和资料

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

## 版权信息

此项目使用 MIT 许可证


