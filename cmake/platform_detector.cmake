
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# platform_detector.cmake for Simple-XX/SimpleKernel.

if (PLATFORM STREQUAL x86_64)
    set(SimpleKernelPlatform x86_64)
    set(SimpleKernelPlatformMacro PLATFORM_x86_64)
elseif (PLATFORM STREQUAL raspi4)
    set(SimpleKernelPlatform raspi4)
    set(SimpleKernelPlatformMacro PLATFORM_RASPI4)
elseif (PLATFORM STREQUAL raspi2)
    set(SimpleKernelPlatform raspi2)
    set(SimpleKernelPlatformMacro PLATFORM_RASPI2)
elseif (PLATFORM STREQUAL bochs)
    set(SimpleKernelPlatform bochs)
    set(SimpleKernelPlatformMacro PLATFORM_BOCHS)
elseif (PLATFORM STREQUAL qemu)
    set(SimpleKernelPlatform qemu)
    set(SimpleKernelPlatformMacro PLATFORM_QEMU)
else ()
    message(WARNING "unexpected PLATFORM ${PLATFORM}, using default value \"bochs\"")
    set(SimpleKernelPlatform bochs)
    set(SimpleKernelPlatformMacro PLATFORM_BOCHS)
endif ()

message(STATUS "SimpleKernelPlatform is ${SimpleKernelPlatform}")
