[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![last-commit-boot](https://img.shields.io/github/last-commit/Simple-XX/SimpleKernel/boot)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

## 关键词

- kernel
- x86_64, riscv64, aarch64
- osdev
- bare metal
- c++, cmake
- uefi, opensbi

## 简介

提供了各个阶段完成度不同的内核，你可以从自己喜欢的地方开始。

## 新增特性

本分支是 SImpleKernel 的首个分支。在本分支中，完成了构建系统的基础搭建、基本的文档部署与自动化测试，当然还有最重要的，有基于 uefi 的 x86_64 内核与由 opensbi 启动的 riscv64 内核，可以在 qemu 上运行，并实现了简单的屏幕输出。

调用顺序

 - x86_64/aarch64
  uefi->main.cpp:main->arch.cpp:arch_init

- riscv64
  opensbi->boot.S:_start->main.cpp:main->arch.cpp:arch_init

- 构建系统 

  参考 [MRNIU/cmake-kernel](https://github.com/MRNIU/cmake-kernel) 的构建系统，详细解释见 [doc/build_system.md](./doc/build_system.md)

- 基于 gnu-efi 引导的 x86_64 内核

  编译后生成 boot.efi 与 kernel.elf，进入 uefi 环境后首先执行 boot.efi，初始化完成后跳转到 kernel.elf 执行

- 基于 opensbi 引导的 riscv64 内核

  由 opensbi 进行初始化，直接跳转到内核地址，进入内核逻辑时为 S 态

- 基于 doxygen 的文档生成与自动部署

  github action 会将文档部署到 https://simple-xx.github.io/SimpleKernel/ (仅 main 分支)

- 基于 CPM 的第三方资源管理

  在 `3rd.cmake` 中使用 CPM 的功能自动下载、集成第三方资源

- 测试

    支持 单元测试、集成测试、系统测试，引入 gtest 作为测试框架，同时统计了测试覆盖率

- 代码分析

    引入 cppcheck、clang-tidy、sanitize 工具提前发现错误

- 代码格式化

    使用 llvm 风格
    
- docker

    支持使用 docker 构建，详细使用方法见 [doc/docker.md](./doc/docker.md)

## 已支持

见 新增特性

## 依赖

[CPM](https://github.com/cpm-cmake/CPM.cmake)

[CPMLicences.cmake](https://github.com/TheLartians/CPMLicenses.cmake)

[google/googletest](https://github.com/google/googletest)

[opensbi](https://github.com/riscv-software-src/opensbi)

[doxygen](https://www.doxygen.nl/)

[lcov](https://github.com/linux-test-project/lcov)

[gcc](https://gcc.gnu.org/)

[qemu](https://www.qemu.org/)

[cppcheck](https://cppcheck.sourceforge.io/)

[clang-tidy](https://clang.llvm.org/extra/clang-tidy/)

[clang-format](https://clang.llvm.org/docs/ClangFormat.html)

[gnu-efi](https://sourceforge.net/projects/gnu-efi/)

