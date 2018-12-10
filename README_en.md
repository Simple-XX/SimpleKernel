
![Build Status](https://travis-ci.org/MRNIU/SimpleKernel.svg?branch=TODO)
![last-commit](https://img.shields.io/github/last-commit/google/skia.svg)
![languages](https://img.shields.io/github/languages/count/badges/shields.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

[English](https://github.com/MRNIU/SimpleKernel/blob/TODO/README_en.md) | [中文](https://github.com/MRNIU/SimpleKernel/blob/TODO/README.md)
# SimpleKernel

## Abstract

SimpleKernel，一个用来练手的简单内核。提供了各个阶段完成度不同的内核，你可以从自己喜欢的地方开始。

使用的语言：

- x86 汇编
- C
- C++
- Shell
- make

## 开发环境

- Ubuntu Linux

    - Bochs
        编译安装
        
    - i386-elf 交叉编译工具链
        编译安装

- Mac
    - Bochs

            brew install bochs
            
    - i386-elf 交叉编译工具链

            brew install i386-elf-binutils
            brew install i386-elf-gcc

注：
Mac 10.14.1，bochs 2.6.9，i386-elf-binutils 2.31.1，i386-elf-gcc 8.2.0 测试通过。

## RUN

    git clone https://github.com/MRNIU/SimpleKernel.git
    cd SimpleKernel/
    sh setup.sh

在出现的 bochs 命令行中 输入 `c` 即可运行。

运行截图
    
![屏幕快照 2018-12-10 下午8.40.12](https://lh3.googleusercontent.com/-bMiOQG70raM/XA5ijMqVcLI/AAAAAAAAAGU/YB_HvVl8JD4PiKbV1UHfQA4HqeD6xWCxACHMYCw/I/%255BUNSET%255D)
    
![屏幕快照 2018-12-10 下午8.40.38](https://lh3.googleusercontent.com/-Lp2Xea_cLgM/XA5ijHG93RI/AAAAAAAAAGc/wpYAqRo7wGIABAAi6jGz3NRhASB2aOQywCHMYCw/I/%255BUNSET%255D)

![屏幕快照 2018-12-10 下午8.40.50](https://lh3.googleusercontent.com/-DwLqMLIWGps/XA5ijD1nZvI/AAAAAAAAAGY/PeRQd2FN8qoOEGq4LEx1vxgmFCmq8qUYACHMYCw/I/%255BUNSET%255D)

更多 bochs 使用方式请参考 bochs 的相关资料。


## QUICK START

```
├── bochsout.txt bochs output
├── bochsrc.txt bochs configuration file
├── setup.sh
├── simplekernel.img
├── someknowledge
├── src/ source directory
│   ├── Makefile build rules
│   ├── arch/ 架构相关代码
│   │   ├── README.md
│   │   ├── i386/ i386 架构
│   │   │   ├── README.md
│   │   │   ├── boot/ boot code，use multiboot
│   │   │   │   ├── boot.s
│   │   │   │   └── link.ld
│   │   │   ├── clock.c
│   │   │   ├── clock.h
│   │   │   ├── cpu.hpp CPU操作
│   │   │   ├── debug/ debug function
│   │   │   │   └── debug.c
│   │   │   ├── intr/ 中断设置
│   │   │   │   ├── README.md
│   │   │   │   ├── intr.c idt 设置
│   │   │   │   ├── intr.h
│   │   │   │   └── intr_s.s
│   │   │   └── mm/ 内存管理
│   │   │       ├── README.md
│   │   │       ├── gdt.c gdt 设置
│   │   │       ├── gdt.h
│   │   │       ├── gdt_s.s
│   │   │       ├── pmm.c 物理内存管理
│   │   │       ├── pmm.h
│   │   │       ├── vmm.c 虚拟内存管理
│   │   │       └── vmm.h
│   │   └── x64/ x64 架构
│   │       └── TODO
│   ├── include/ 头文件
│   │   ├── DataStructuer/ data structure
│   │   │   ├── BinarySearchTree.cpp
│   │   │   ├── DataStructuer.h
│   │   │   ├── LinkedList.cpp
│   │   │   ├── Queue.cpp 队列
│   │   │   ├── SortAlgorithm.cpp 排序算法
│   │   │   └── Stack.cpp 栈
│   │   ├── README.md
│   │   ├── debug.h
│   │   ├── drv/ 设备头文件
│   │   │   ├── keyboard.h
│   │   │   └── mouse.h
│   │   ├── elf.h elf 格式定义
│   │   ├── kernel.h 内核函数直接引用的头文件
│   │   ├── libc/ c 标准库
│   │   │   ├── README.md
│   │   │   ├── assert.h 断言
│   │   │   ├── stdarg.h
│   │   │   ├── stdbool.h
│   │   │   ├── stddef.h
│   │   │   ├── stdint.h
│   │   │   ├── stdio/ 标准输入输出
│   │   │   │   ├── printk.c
│   │   │   │   └── vsprintf.c
│   │   │   ├── stdio.h
│   │   │   ├── string/ 字符串处理
│   │   │   │   └── string.c
│   │   │   ├── string.h
│   │   ├── mm/ 内存相关头文件
│   │   │   ├── README.md
│   │   │   └── mm.h
│   │   ├── multiboot.h 多重引导规范定义
│   │   ├── pic.hpp 8259A 中断芯片设置
│   │   ├── port.hpp 端口操作
│   │   ├── tty.hpp tty 定义
│   │   └── vga.hpp vga 显示定义
│   └── kernel/
│       ├── README.md
│       ├── drv/ 设备
│       │   ├── kb.c
│       │   └── mouse.c
│       └── kernel.c 内核入口
└── tools/ 工具，在 .rb 文件中你可以找到 gcc 和 binutils 的编译选项
    ├── i386-elf-binutils.rb
    └── i386-elf-gcc.rb
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
- 键盘输入处理
- 鼠标输入处理
- debug 函数
- 添加编码规范测试

## 作者

作者：[MRNIU](https://github.com/MRNIU)

您也可以在贡献者名单中参看所有参与该项目的开发者。

## 贡献

请阅读 CONTRIBUTING.md 查阅为该项目做出贡献的开发者。

## 鸣谢

此项目参考了很多优秀的项目和资料

[osdev](https://wiki.osdev.org)

[GRUB 在 Mac 上的安装](https://wiki.osdev.org/GRUB#Installing_GRUB_2_on_OS_X)

[JamesM's kernel development tutorials](http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html)

[xOS](https://github.com/fengleicn/xOS)

[hurlex](http://wiki.0xffffff.org/posts/hurlex-8.html)

[howerj/os](https://github.com/howerj/os)

[cfenollosa/os-tutorial](https://github.com/cfenollosa/os-tutorial)

[omarrx024/xos](https://github.com/omarrx024/xos)

[PurpleBooth/project-title](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2%23project-title)

[0xAX weblong](http://0xax.blogspot.com/search/label/asm)

## 版权信息

此项目使用 MIT 许可证


