
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# arch_detector.cmake for Simple-XX/SimpleKernel.

if (ARCH STREQUAL i386)
    set(SimpleKernelArch i386)
elseif (ARCH STREQUAL x86_64)
    set(SimpleKernelArch x86_64)
elseif (ARCH STREQUAL arm)
    set(SimpleKernelArch arm)
elseif (ARCH STREQUAL riscv64)
    set(SimpleKernelArch riscv64)
else ()
    message(WARNING "unexpected ARCH ${ARCH}, using default value \"x86_64\"")
    set(SimpleKernelArch i386)
endif ()

message(STATUS "SimpleKernelArch is ${SimpleKernelArch}")
