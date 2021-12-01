
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 对 shell 传入的 ARCH 参数进行处理

# TODO: 优化 CMake 流程，环境搭建由自动脚本实现

# 如果 ARCH 为 i386 或 x86_64，统一添加 ia32 前缀
if (ARCH STREQUAL i386)
    set(SimpleKernelArch ia32/i386)
elseif (ARCH STREQUAL x86_64)
    set(SimpleKernelArch ia32/x86_64)
# 其它情况不变
elseif (ARCH STREQUAL aarch64)
    set(SimpleKernelArch aarch64)
elseif (ARCH STREQUAL riscv64)
    set(SimpleKernelArch riscv64)
else ()
    # 不支持的 ARCH
    message(WARNING "unexpected ARCH: ${ARCH}, using default value \"riscv64\"")
    # 默认设为 riscv64
    set(SimpleKernelArch riscv64)
endif ()

# 输出 SimpleKernelArch
message(STATUS "SimpleKernelArch is ${SimpleKernelArch}")
