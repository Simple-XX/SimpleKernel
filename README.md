[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![last-commit-boot](https://img.shields.io/github/last-commit/Simple-XX/SimpleKernel/boot)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

boot branch

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

本分支是 SImpleKernel 的首个分支。在本分支中，完成了构建系统的基础搭建、基本的文档部署与自动化测试，当然还有最重要的，有基于 uefi 的 x86_64 内核与由 opensbi 启动的 riscv64 内核，可以在 qemu 上运行，并实现了简单的屏幕输出。

||x86_64|riscv64|aarch64|
| :-----------------------: | :-------------------------------: | :---------------------------------------------: | :-------------------: |
|引导|UEFI|opensbi|UEFI(未完成)|
|基本输出|通过 serial 实现|通过 opensbi 提供的 ecall 实现|TODO|
|硬件资源探测|由 UEFI 传递|dtb 解析|TODO|

- 构建系统

  参考 [MRNIU/cmake-kernel](https://github.com/MRNIU/cmake-kernel) 的构建系统，详细解释见 [doc/build_system.md](./doc/build_system.md)

- libc 支持

  |     函数/变量名      |               用途               |      |
  | :------------------: | :------------------------------: | :--: |
  | `__stack_chk_guard`  |              栈保护              |      |
  | `__stack_chk_fail()` |       栈保护检查失败后调用       |      |
  |      `memcpy()`      |            复制内存块            |      |
  |     `memmove()`      |  复制内存块，可以处理重叠区域。  |      |
  |      `memset()`      |            设置内存块            |      |
  |      `memcmp()`      |            比较内存块            |      |
  |      `memchr()`      |        在内存块中查找字符        |      |
  |      `strcpy()`      |            复制字符串            |      |
  |     `strncpy()`      |       复制指定长度的字符串       |      |
  |      `strcat()`      |            连接字符串            |      |
  |      `strcmp()`      |            比较字符串            |      |
  |     `strncmp()`      |       比较指定长度的字符串       |      |
  |      `strlen()`      |          获取字符串长度          |      |
  |     `strnlen()`      |        获取指定字符串长度        |      |
  |      `strchr()`      |   查找字符在字符串中的首次出现   |      |
  |     `strrchr()`      | 反向查找字符在字符串中的首次出现 |      |

- libc++ 支持

    |       函数/变量名       |                 用途                 |      |
    | :---------------------: | :----------------------------------: | :--: |
    |    `__cxa_atexit()`     |             注册析构函数             |      |
    |   `__cxa_finalize()`    |             调用析构函数             |      |
    | `__cxa_guard_acquire()` |         静态局部变量初始化锁         |      |
    | `__cxa_guard_release()` |        静态局部变量初始化完成        |      |
    |  `__cxa_guard_abort()`  |        静态局部变量初始化出错        |      |
    |    `__cxa_rethrow()`    |         用于简单处理 `throw`         |      |
    |    `operator new()`     | 运算符重载，空实现，用于全局对象支持 |      |
    |   `operator new[]()`    | 运算符重载，空实现，用于全局对象支持 |      |
    |    `operator new()`     | 运算符重载，空实现，用于全局对象支持 |      |
    |   `operator new[]()`    | 运算符重载，空实现，用于全局对象支持 |      |
    |   `operator delete()`   | 运算符重载，空实现，用于全局对象支持 |      |
    |   `operator delete()`   | 运算符重载，空实现，用于全局对象支持 |      |
    |  `operator delete[]()`  | 运算符重载，空实现，用于全局对象支持 |      |
    |  `operator delete[]()`  | 运算符重载，空实现，用于全局对象支持 |      |

- 打印函数调用栈

  逐层回溯帧指针后与 elf 信息进行对比，实现对函数调用栈的打印

- 基础 c++ 异常 支持

  通过 throw 抛出异常后停机，没有上下文相关的处理

- 带颜色的输出

  基于 ANSI 转义码，在支持 ANSI 转义码的终端中可以显示有颜色的字符串

- 基于 gnu-efi 引导的 x86_64 内核

  编译后生成 boot.efi 与 kernel.elf，进入 uefi 环境后首先执行 boot.efi，初始化完成后跳转到 kernel.elf 执行

- 基于 opensbi 引导的 riscv64 内核

  1. 由 opensbi 进行初始化，直接跳转到内核地址，进入内核逻辑时为 S 态
  2. gp 寄存器的初始化
  3. 使用 libfdt(dtc 的一部分) 对 opensbi 传递的 dtb 信息进行解析
  4. ns16550a 串口驱动
  5. 基于 opensbi 的 printf

- 全局对象

  |                  对象名                  |                 位置                 |          用途           |
  | :--------------------------------------: | :----------------------------------: | :---------------------: |
  |          `static ostream cout`           |  src/kernel/libcxx/include/iostream  | 内核中的 std::cout 实现 |
  | `static Singleton<KernelElf> kKernelElf` |  src/kernel/include/kernel_elf.hpp   | 解析内核自身的 elf 信息 |
  | `static Singleton<KernelFdt> kKernelFdt` |  src/kernel/include/kernel_fdt.hpp   |      解析 dtb 信息      |
  | `static Singleton<BasicInfo> kBasicInfo` |     src/kernel/include/kernel.h      |      内核基本信息       |
  | `static cpu::Serial kSerial(cpu::kCom1)` | src/kernel/arch/x86_64/arch_main.cpp |     X86_64 下的串口     |

- 基于 doxygen 的文档生成与自动部署

  github action 会将文档部署到 https://simple-xx.github.io/SimpleKernel/ (仅 main 分支)

- 基于 git submodule 的第三方资源管理

  使用 git submodule 集成第三方资源

- 测试

    支持 单元测试、集成测试、系统测试，引入 gtest 作为测试框架，同时统计了测试覆盖率

- 代码分析

    引入 cppcheck、clang-tidy、sanitize 工具提前发现错误

- 代码格式化

    使用 google 风格

- docker

    支持使用 docker 构建，详细使用方法见 [doc/docker.md](./doc/docker.md)

## 已支持特性

见 新增特性

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
