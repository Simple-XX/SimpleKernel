
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# arch_detector.cmake for Simple-XX/SimpleKernel.

if (ARCH STREQUAL x86_64)
    set(SimpleKernelArch x86_64)
elseif (ARCH STREQUAL raspi2)
    set(SimpleKernelArch RASPI2)
else ()
    message(WARNING "unexpected ARCH ${ARCH}, using default value \"x86_64\"")
    set(SimpleKernelArch x86_64)
endif ()

message(STATUS "SimpleKernelArch is ${SimpleKernelArch}")
