![Build Status](https://travis-ci.org/MRNIU/SimpleKernel.svg?branch=TODO)
![last-commit](https://img.shields.io/github/last-commit/google/skia.svg)
![languages](https://img.shields.io/github/languages/count/badges/shields.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![HitCount](http://hits.dwyl.io/MRNIU/SimpleKernel.svg)](http://hits.dwyl.io/MRNIU/SimpleKernel)

[English](https://github.com/MRNIU/SimpleKernel/blob/TODO/README_en.md) | [中文](https://github.com/MRNIU/SimpleKernel/blob/TODO/README.md)

# SimpleKernel

## 关键词

- Kernel
- C/C++

- UXIN-like
- Multiboot2

## 简介

SimpleKernel，一个用来练手的简单内核。提供了各个阶段完成度不同的内核，你可以从自己喜欢的地方开始。

## 开发环境

- Ubuntu Linux

    - bochs
    - i386-elf tool chain
    i386-elf-binutils, i386-elf-gcc, i386-elf-grub
    - xorriso
    
- Mac

    - Homebrew

        https://brew.sh

    - bochs

    - i386-elf tool chain

        i386-elf-binutils, i386-elf-gcc, i386-elf-grub

    - xorriso

注：可以使用 setup.sh 自动安装环境

## 如何运行

```shell
git clone https://github.com/MRNIU/SimpleKernel.git
cd SimpleKernel/
sh setup.sh
```

运行截图

![](https://ws3.sinaimg.cn/large/006tKfTcly1g16s317uw8j316r0u0tkt.jpg)

## 目录结构

- 原则

    整个工程按照功能模块划分子目录，每个子目录再划分头文件和源文件目录，以便架构清晰、易懂。

### 目录设计
- 原则

    目录的命名能准确描述模块的基本功能，建议用小写字母且不含下划线、点等特殊符号；

    目录必须放于相包含的父目录之下，并需要明确与其他目录间的耦合性。

- 示例

	kernel：系统内核部分；
	libs：依赖库；

### 依赖关系
- 原则

    新添加组件往往依赖于系统原有组件，必须以最小耦合的方式明确所直接依赖的组件。

### 头文件

#### 文件命名

​	头文件命名能准确描述文件所包含的模块内容，达到通俗、易懂的目的。

## Makefile

## 测试

### 自动集成

合并到 master 分支时需要通过 Travis CI 测试。编译没有错误即可。

## 代码风格

## 镜像文件

simplekernel.img 是 1.44 软盘，我们的内核就在这里。

## TODO

- 修复 bug
- debug 函数
- 添加编码规范测试
- 并发
- 文件系统
- 设备驱动
- 虚拟内存管理

## 贡献者

[MRNIU](https://github.com/MRNIU)

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

## 支持作者

请我喝杯咖啡吧！

![](https://tva1.sinaimg.cn/large/006tNbRwly1g9yjfoboa4j30go0p0411.jpg)

![](https://tva1.sinaimg.cn/large/006tNbRwly1g9yjg7p0auj30u014qn7q.jpg)

## 版权信息

此项目使用 MIT 许可证
