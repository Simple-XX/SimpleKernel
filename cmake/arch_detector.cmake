
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# arch_detector.cmake for Simple-XX/SimpleKernel.

# TODO: 优化 CMake 流程，环境搭建由自动脚本实现

if (ARCH STREQUAL i386)
    set(SimpleKernelArch i386)
elseif (ARCH STREQUAL x86_64)
    set(SimpleKernelArch x86_64)
elseif (ARCH STREQUAL arm)
    set(SimpleKernelArch arm)
elseif (ARCH STREQUAL riscv64)
    set(SimpleKernelArch riscv64)
else ()
    message(WARNING "unexpected ARCH: ${ARCH}, using default value \"riscv64\"")
    set(SimpleKernelArch riscv64)
endif ()

message(STATUS "SimpleKernelArch is ${SimpleKernelArch}")
