
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# arch_detector.cmake for Simple-XX/SimpleKernel.

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
