[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![last-commit-boot](https://img.shields.io/github/last-commit/Simple-XX/SimpleKernel/intr)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

intr branch

## 关键词

- kernel
- x86_64, riscv64, aarch64
- osdev
- bare metal
- c++, cmake
- uefi, opensbi

## 简介

提供了各个阶段完成度不同的内核，你可以从自己喜欢的地方开始。

## 快速开始

1. 需要 Ubuntu 环境

2. 安装依赖

    ```shell
    sudo apt install --fix-missing -y doxygen graphviz clang-format clang-tidy cppcheck qemu-system lcov gdb-multiarch libgtest-dev cmake gcc g++ gcc-riscv64-linux-gnu g++-riscv64-linux-gnu gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
    ```

3. 编译并运行

    ```shell
    cd SimpleKernel
    # 支持三种架构
    cmake --preset build_riscv64
    # cmake --preset build_x86_64
    # cmake --preset build_aarch64
    # 进入其中一个架构
    cd build_riscv64
    # 在 qemu 中运行
    make run_run
    ```

4. 调试

    ```shell
    # 进入构建目录后执行
    make run_debug
    ```
    
    在一个新的 shell 中进入 gdb
    ```shell
    # 进入 gdb
    gdb-multiarch
    # 连接端口
    target remote :1234
    # 加载符号表
    file image/kernel.elf
    # 开始执行
    c
    ```

## 新增特性

- riscv64

    1. 对 CSR 寄存器的抽象

    2. 寄存器状态打印

    3. 基于 Direct 的中断处理
    
    4. 中断注册函数

    5. 时钟中断

- TODO

    riscv64 外部中断

    riscv64 键盘中断

    x86_64 cpu 抽象

    x86_64 中断处理

    x86_64 时钟中断

    x86_64 键盘中断

## 已支持特性

  - [x] [BUILD] 使用 CMake 的构建系统

  - [x] [BUILD] 使用 gdb remote 调试

  - [x] [BUILD] 第三方资源集成

  - [x] [COMMON] C++ 全局对象的构造

  - [x] [COMMON] printf 支持

  - [x] [x86_64] 基于 gnuefi 的 bootloader

  - [x] [x86_64] 基于 serial 的基本输出

  - [x] [x86_64] 物理内存信息探测

  - [x] [x86_64] 显示缓冲区探测

  - [x] [riscv64] gp 寄存器的初始化

  - [x] [riscv64] 基于 opensbi 的基本输出

  - [x] [riscv64] device tree 硬件信息解析
  
  - [x] [riscv64] ns16550a 串口驱动

  - [ ] [aarch64] 基于 gnuefi 的 bootloader(调试中)

  


## 使用的第三方资源

[CPM](https://github.com/cpm-cmake/CPM.cmake)

[opensbi](https://github.com/riscv-software-src/opensbi)

[gnu-efi](https://sourceforge.net/projects/gnu-efi/)

[gdbinit](https://github.com/gdbinit/Gdbinit)

[opensbi_interface](https://github.com/MRNIU/opensbi_interface)

[printf_bare_metal](https://github.com/MRNIU/printf_bare_metal)

[fdt_parser](https://github.com/MRNIU/fdt_parser)

[CPMLicences.cmake](https://github.com/TheLartians/CPMLicenses.cmake)

[google/googletest](https://github.com/google/googletest)

## 开发指南

代码风格：Google，已由 .clang-format 指定

命名规范：[Gooele 开源项目风格指南](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/contents.html)
