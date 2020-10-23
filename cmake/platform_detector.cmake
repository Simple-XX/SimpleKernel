
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# platform_detector.cmake for Simple-XX/SimpleKernel.

if (PLATFORM STREQUAL physical)
    set(SimpleKernelPlatform physical)
    set(SimpleKernelPlatformMacro PHY)
elseif (PLATFORM STREQUAL bochs)
    set(SimpleKernelPlatform bochs)
    set(SimpleKernelPlatformMacro BOCHS)
elseif (PLATFORM STREQUAL qemu)
    set(SimpleKernelPlatform qemu)
    set(SimpleKernelPlatformMacro QEMU)
else ()
    message(WARNING "Unexpected PLATFORM ${PLATFORM}, using default value \"bochs\"")
    set(PLATFORM bochs)
    set(SimpleKernelPlatform bochs)
    set(SimpleKernelPlatformMacro BOCHS)
endif ()

message(STATUS "SimpleKernelPlatform is ${SimpleKernelPlatform}")
