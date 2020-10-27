
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# platform_detector.cmake for Simple-XX/SimpleKernel.

if (PLATFORM STREQUAL x86_64)
    set(SimpleKernelPlatform x86_64)
    set(SimpleKernelPlatformMacro PLATFORM_x86_64)
elseif (PLATFORM STREQUAL bochs)
    set(SimpleKernelPlatform bochs)
    set(SimpleKernelPlatformMacro PLATFORM_BOCHS)
elseif (PLATFORM STREQUAL qemu)
    set(SimpleKernelPlatform qemu)
    set(SimpleKernelPlatformMacro PLATFORM_QEMU)
else ()
    message(WARNING "Unexpected PLATFORM ${PLATFORM}, using default value \"bochs\"")
    set(PLATFORM bochs)
    set(SimpleKernelPlatform bochs)
    set(SimpleKernelPlatformMacro PLATFORM_BOCHS)
endif ()

message(STATUS "SimpleKernelPlatform is ${SimpleKernelPlatform}")
