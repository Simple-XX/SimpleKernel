[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![last-commit-boot](https://img.shields.io/github/last-commit/Simple-XX/SimpleKernel/boot)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

## Key Words

- kernel
- x86_64, riscv64, aarch64
- osdev
- bare metal
- c++, cmake
- uefi, opensbi

## Introduction

SimpleKernel, a simple kernel for learning. Contains the basic functionality of an operating system

Kernels with different levels of completion are available, and you can start from where you like.

## What's NEW

This branch is the first branch of SImpleKernel. In this branch, the foundation of the build system is completed, basic documentation deployment and automated testing, and of course the most important, there is a uefi based x86_64 kernel and riscv64 kernel started by opensbi, which can run on qemu, and achieve simple screen output.

- Control flow

  - x86_64/aarch64

      <img src='https://g.gravizo.com/svg?
      @startuml;
      uefi_shell->efi_main:boot.cpp;
      efi_main->_start:main.cpp;
      _start->cpp_init:libcxx.cpp;
      cpp_init->main:main.cpp;
      main->arch_init:arch.cpp;
      arch_init->main;
      main->main;
      @enduml
      '>

  - riscv64

    <img src='https://g.gravizo.com/svg?
      @startuml;
      opensbi->_boot:boot.S;
      _boot->_start:main.cpp;
      _start->cpp_init:libcxx.cpp;
      cpp_init->main:main.cpp;
      main->arch_init:arch_init.cpp;
      arch_init->main;
      main->main;
      @enduml
    '>

- Build system

  Reference [MRNIU/cmake-kernel](https://github.com/MRNIU/cmake-kernel) build system, a detailed explanation see [doc/build_system.md](./doc/build_system.md)

- x86_64 kernel based on gnu-efi boot

  After compiling, boot.efi and kernel.elf are generated. After entering uefi environment, boot.efi is executed first. After initialization, kernel.elf is executed

- riscv64 kernel based on opensbi boot

  Initializing by opensbi, it jumps directly to the kernel address and enters the S state when entering the kernel logic

- Doxygen-based document generation and automatic deployment

  Making the action will document deployment to https://simple-xx.github.io/SimpleKernel/ (the main branch only)

- Third-party resource management based on CPM

  Use CPM's functionality in '3rd.cmake' to automatically download and integrate third-party resources

- Test

    Support unit testing, integration testing, system testing, the introduction of gtest as a test framework, while the test coverage statistics

- Code analysis

    Introduce cppcheck, clang-tidy, and sanitize tools to detect errors in advance

- Code formatting

    Use the llvm style
    
- docker

    Supports building with docker, see [doc/docker.md](./doc/docker.md)

## Supported

See What's NEW

## 3rd

[CPM](https://github.com/cpm-cmake/CPM.cmake)

[opensbi](https://github.com/riscv-software-src/opensbi)

[gnu-efi](https://sourceforge.net/projects/gnu-efi/)

[gdbinit](https://github.com/gdbinit/Gdbinit)

[opensbi_interface](https://github.com/MRNIU/opensbi_interface)

[printf_bare_metal](https://github.com/MRNIU/printf_bare_metal)

[fdt_parser](https://github.com/MRNIU/fdt_parser)

[CPMLicences.cmake](https://github.com/TheLartians/CPMLicenses.cmake)

[google/googletest](https://github.com/google/googletest)
