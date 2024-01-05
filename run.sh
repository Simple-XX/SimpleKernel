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

# TARGET_ARCH=x86_64
TARGET_ARCH=riscv64
# TARGET_ARCH=aarch64

# 删除旧文件
# rm -rf build_${TARGET_ARCH}
# 生成
cmake --preset build_${TARGET_ARCH}
# 编译并运行
cmake --build build_${TARGET_ARCH} --target run_run
