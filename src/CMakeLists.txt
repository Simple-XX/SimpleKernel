
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# Set C flags for arch
if (SimpleKernelArch STREQUAL x86)
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -mno-sse -m32")
elseif (SimpleKernelArch STREQUAL x86_64)
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -mno-sse -m32")
else ()
    message(FATAL_ERROR "unexpected ARCH ${SimpleKernelArch}")
endif ()

# Set C flags for debug
if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif ()

# Set common flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -std=gnu11 -Wall -Wextra -g -nostdlib -nostdinc -fno-exceptions -nostartfiles -fno-builtin -O2 -D${SimpleKernelPlatformMacro}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

message(STATUS "CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_ASM_FLAGS is ${CMAKE_ASM_FLAGS}")
message(STATUS "TOOLCHAIN_PREFIX is ${TOOLCHAIN_PREFIX}")
message(STATUS "CMAKE_OBJCOPY is ${CMAKE_OBJCOPY}")

# Sub dirs
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/libc)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/arch)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/kernel)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/drv)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/test)

# Link
add_executable(${KernelName}
        $<TARGET_OBJECTS:libc>
        $<TARGET_OBJECTS:boot>
        $<TARGET_OBJECTS:gdt>
        $<TARGET_OBJECTS:intr>
        $<TARGET_OBJECTS:debug>
        $<TARGET_OBJECTS:pmm>
        $<TARGET_OBJECTS:heap>
        $<TARGET_OBJECTS:kernel>
        $<TARGET_OBJECTS:mem>
        $<TARGET_OBJECTS:8259A>
        $<TARGET_OBJECTS:clock>
        $<TARGET_OBJECTS:vga>
        $<TARGET_OBJECTS:console>
        $<TARGET_OBJECTS:keyboard>
        $<TARGET_OBJECTS:test>)

target_link_options(${KernelName} PRIVATE -T ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/boot/link32.ld)
target_link_options(${KernelName} PRIVATE -Wl,-melf_i386)
