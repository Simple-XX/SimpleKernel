#!/bin/bash

# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# run.sh for Simple-XX/SimpleKernel.
# 在虚拟机中运行内核

# shell 执行出错时终止运行
set -e
# 输出实际执行内容
#set -x

MACHINE=qemu

# ARCH=riscv64
ARCH=x86_64
# ARCH=aarch64

# 指定要使用的 efi
# @todo 目前只支持使用 posix-uefi
USE_GNU_UEFI=0

CMAKE_BUILD_TYPE=Debug
#CMAKE_BUILD_TYPE=Release

GENERATOR=make
#GENERATOR=ninja

COMPILER=gcc
#COMPILER=clang

# 重新编译
mkdir -p ./build_${ARCH}/
cd ./build_${ARCH}
cmake \
  -DMACHINE=${MACHINE} \
  -DARCH=${ARCH} \
  -DUSE_GNU_UEFI=${USE_GNU_UEFI} \
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
  -DCOMPILER=${COMPILER} \
  -DGENERATOR=${GENERATOR} \
  ..
make run
