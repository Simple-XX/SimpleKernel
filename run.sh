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

# CMAKE_SYSTEM_PROCESSOR=x86_64
CMAKE_SYSTEM_PROCESSOR=riscv64
# CMAKE_SYSTEM_PROCESSOR=aarch64

# 删除旧文件
# rm -rf build_${CMAKE_SYSTEM_PROCESSOR}
# 生成
cmake --preset build_${CMAKE_SYSTEM_PROCESSOR}
# 编译并运行
cmake --build build_${CMAKE_SYSTEM_PROCESSOR} --target run_run
