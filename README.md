[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![last-commit-boot](https://img.shields.io/github/last-commit/Simple-XX/SimpleKernel/intr)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

memory branch

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

1. 需要 Ubuntu 环境或使用 docker(见 tools/docker.md)

2. 安装依赖

    ```shell
    sudo apt install --fix-missing -y gcc g++ gcc-riscv64-linux-gnu g++-riscv64-linux-gnu gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
    sudo apt install --fix-missing -y cmake qemu-system gdb-multiarch
    sudo apt install --fix-missing -y doxygen graphviz
    sudo apt install --fix-missing -y doxygen graphviz 
    sudo apt install --fix-missing -y clang-format clang-tidy cppcheck libgtest-dev lcov
    ```

    ```shell
    git submodule update --init --recursive
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
    # 编译内核
    make kernel
    # 在 qemu 中运行
    make run
    ```

4. 调试

    ```shell
    # 进入构建目录后执行
    make debug
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

## 执行流

  - x86_64/aarch64

      <img src='https://g.gravizo.com/svg?
      @startuml;
      uefi_shell->efi_main:boot.cpp;
      efi_main->_start:main.cpp;
      _start->CppInit:LibCxxInit.cpp;
      CppInit->main:main.cpp;
      main->ArchInit:arch.cpp;
      ArchInit->main;
      main->main;
      @enduml
      '>

  - riscv64

    <img src='https://g.gravizo.com/svg?
      @startuml;
      opensbi->_boot:boot.S;
      _boot->_start:main.cpp;
      _start->CppInit:LibCxxInit.cpp;
      CppInit->main:main.cpp;
      main->ArchInit:ArchInit.cpp;
      ArchInit->main;
      main->main;
      @enduml
    '>


## 新增特性

- riscv64

    1. 
    
- X86_64

    1. 
    
- TODO

    

- 全局对象

  |                  对象名                  |                 位置                 |          用途           |
  | :--------------------------------------: | :----------------------------------: | :---------------------: |
  |          `static ostream cout`           |  src/kernel/libcxx/include/iostream  | 内核中的 std::cout 实现 |
  | `static Singleton<KernelElf> kKernelElf` |  src/kernel/include/kernel_elf.hpp   | 解析内核自身的 elf 信息 |
  | `static Singleton<KernelFdt> kKernelFdt` |  src/kernel/include/kernel_fdt.hpp   |      解析 dtb 信息      |
  | `static Singleton<BasicInfo> kBasicInfo` |  src/kernel/include/basic_info.hpp   |      内核基本信息       |
  | `static cpu::Serial kSerial(cpu::kCom1)` | src/kernel/arch/x86_64/arch_main.cpp |     X86_64 下的串口     |

  

## 已支持特性

  - [x] [BUILD] 使用 CMake 的构建系统
  - [x] [BUILD] 使用 gdb remote 调试
  - [x] [BUILD] 第三方资源集成
  - [x] [COMMON] C++ 全局对象的构造
  - [x] [COMMON] C++ 静态局部对象构造
  - [x] [COMMON] C 栈保护支持
  - [x] [COMMON] printf 支持
  - [x] [COMMON] 简单的 C++ 异常支持
  - [x] [COMMON] 带颜色的字符串输出
  - [x] [x86_64] 基于 gnuefi 的 bootloader
  - [x] [x86_64] 基于 serial 的基本输出
  - [x] [x86_64] 物理内存信息探测
  - [x] [x86_64] 显示缓冲区探测
  - [x] [x86_64] 调用栈回溯
  - [x] [riscv64] gp 寄存器的初始化
  - [x] [riscv64] 基于 opensbi 的基本输出
  - [x] [riscv64] device tree 硬件信息解析
  - [x] [riscv64] ns16550a 串口驱动
  - [x] [riscv64] 调用栈回溯(仅打印地址)
  - [ ] [aarch64] 基于 gnuefi 的 bootloader(调试中)
  - [x] [riscv64] 对 CSR 寄存器的抽象
  - [x] [riscv64] 寄存器状态打印
  - [x] [riscv64] 基于 Direct 的中断处理
  - [x] [riscv64] 中断注册函数
  - [x] [riscv64] 时钟中断
  - [x] [x86_64] cpu 抽象
  - [x] [x86_64] 8259A pic 控制器抽象
  - [x] [x86_64] 8253/8254 timer 控制器抽象
  - [x] [x86_64] gdt 初始化
  - [x] [x86_64] 中断处理流程
  - [x] [x86_64] 中断注册函数
  - [x] [x86_64] 时钟中断

## 使用的第三方资源

[opensbi](https://github.com/riscv-software-src/opensbi)

[gnu-efi](https://sourceforge.net/projects/gnu-efi/)

[gdbinit](https://github.com/gdbinit/Gdbinit)

[opensbi_interface](https://github.com/MRNIU/opensbi_interface)

[printf_bare_metal](https://github.com/MRNIU/printf_bare_metal)

[dtc](https://git.kernel.org/pub/scm/utils/dtc/dtc.git)

[google/googletest](https://github.com/google/googletest)

## 开发指南

代码风格：Google，已由 .clang-format 指定

命名规范：[Gooele 开源项目风格指南](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/contents.html)
