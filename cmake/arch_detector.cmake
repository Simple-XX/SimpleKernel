
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# arch_detector.cmake for SimpleXX/SimpleKernel.

if (ARCH STREQUAL x86_64)
    set(SimpleKernelArch x86_64)
else ()
    message(WARNING "Unexpected ARCH ${ARCH}, using default value \"x86_64\"")
    set(ARCH x86_64)
    set(SimpleKernelArch x86_64)
endif ()

message(STATUS "SimpleKernelArch is ${SimpleKernelArch}")
