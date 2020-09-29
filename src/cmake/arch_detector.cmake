
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# arch_detector.cmake for SimpleXX/SimpleKernel.

if (ARCH STREQUAL x86)
    set(SimpleKernelArch x86)
elseif (ARCH STREQUAL x86_64)
    set(SimpleKernelArch x86_64)
else ()
    message(WARNING "unexpected ARCH ${ARCH}, using default value \"x86\"")
    set(SimpleKernelArch x86)
    set(ARCH x86\(default\))
endif ()

message(STATUS "SimpleKernelArch is ${SimpleKernelArch}")
