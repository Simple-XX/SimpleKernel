![github ci](https://github.com/Simple-XX/SimpleKernel/workflows/CMake/badge.svg) ![last-commit](https://img.shields.io/github/last-commit/google/skia.svg) ![languages](https://img.shields.io/github/languages/count/badges/shields.svg) ![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg) [![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE) [![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](https://github.com/Simple-XX/SimpleKernel/blob/master/README_en.md) | [中文](https://github.com/Simple-XX/SimpleKernel/blob/master/README.md)

# SimpleKernel

## 关键词

- kernel
- x86, x86_64, riscv64
- C, C++, AT&T ASM
- cmake
- multiboot2, opensbi

## 简介

提供了各个阶段完成度不同的内核，你可以从自己喜欢的地方开始。

各分支内容：

1. boot: 系统的启动，从引导程序到内核入口
2. printf: 基本的字符输出，便于调试
3. parse_boot_info: 对引导程序传递信息的初步解析
4. pmm: 物理内存初始化
5. vmm: 虚拟内存初始化
6. heap: 堆管理
7. lib: C++ std/stl 的部分支持
8. intr: 中断管理
9. 进程: TODO
10. 文件系统: TODO
11. 设备管理: TODO
12. 系统调用: TODO
13. 用户模式: TODO

## 开发环境

- 基本工具

    交叉编译器 `x86_64-elf-gcc`, `riscv64-unknown-elf-gcc`, `arm-none-eabi-gcc`

    调试工具 `x86_64-elf-gdb`, `riscv64-unknown-elf-gdb`, `arm-none-eabi-gdb`

    构建工具 `cmake`

    模拟器 `bochs`, `qemu`

- For x86

    引导程序 `grub`
    
    制作内核镜像 `xorriso`
    
- For riscv64

    引导程序 `opensbi`
    
- For arm

    TODO

## 如何运行

```shell
git clone https://github.com/Simple-XX/SimpleKernel.git
cd SimpleKernel/
bash ./run.sh
```

运行截图

![](https://tva1.sinaimg.cn/large/00831rSTly1gdl6j8bxw7j317s0u0td9.jpg)

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

头文件命名能准确描述文件所包含的模块内容，达到通俗、易懂的目的。

## CMake

## 测试

### 自动集成

每次 push 会使用 Github Action 进行测试，可以通过编译即可。

## 代码风格

- git commit 规范：

    [tools/Git Commit 规范.pdf](./tools/Git Commit 规范.pdf)

- 代码样式

    由 .clang-format 指定

## TODO

- 并发
- 文件系统
- 设备驱动

## 贡献者

[MRNIU](https://github.com/MRNIU)

[cy295957410](https://github.com/cy295957410)

[rakino](https://github.com/rakino)

[xiaoerlaigeid](https://github.com/xiaoerlaigeid)

[digmouse233](https://github.com/digmouse233)

[KehRoche](https://github.com/KehRoche)

## 贡献

Free to PR!

## 感谢

此项目参考了很多优秀的项目和资料。

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

## 捐助者

- [digmouse233](https://github.com/digmouse233)

- l*e

## 版权信息

此项目使用 MIT 许可证
