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

ARCH=riscv64
#ARCH=x86_64
#ARCH=aarch64

# 重新编译
mkdir -p ./build_${ARCH}/
cd ./build_${ARCH}
cmake -DARCH=${ARCH} ..
make run
