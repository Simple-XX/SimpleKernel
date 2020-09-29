
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.18)

# skip cmake compiler check
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

project(SimpleKernel LANGUAGES C ASM)

if(${SimpleKernel_SOURCE_DIR} STREQUAL ${SimpleKernel_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Set debug
set(CMAKE_BUILD_TYPE Debug)

# Set C gnu11
set(CMAKE_C_STANDARD 11)

# Set cmake moudle path
set(CMAKE_MODULE_PATH "${SimpleKernel_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Set arch
include(${SimpleKernel_SOURCE_DIR}/cmake/arch_detector.cmake)
# Set platform
include(${SimpleKernel_SOURCE_DIR}/cmake/platform_detector.cmake)
# Set header files
include(${SimpleKernel_SOURCE_DIR}/cmake/header_files.cmake)
# Find asm source files
include(${SimpleKernel_SOURCE_DIR}/cmake/find_asm_files.cmake)

message(STATUS "PLATFORM is ${PLATFORM}")

# Set C flags for arch
if (SimpleKernelArch STREQUAL x86)
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -mno-sse -m32")
elseif (SimpleKernelArch STREQUAL x86_64)
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -mno-sse -m32")
else ()
    message(FATAL_ERROR "unexpected ARCH ${SimpleKernelArch}")
endif ()
message(STATUS "ARCH is ${ARCH}")

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set kernel name
set(KernelName kernel.bin)

# Sub dirs
add_subdirectory(${SimpleKernel_SOURCE_DIR}/libc)
add_subdirectory(${SimpleKernel_SOURCE_DIR}/ds_alg)
add_subdirectory(${SimpleKernel_SOURCE_DIR}/arch)
add_subdirectory(${SimpleKernel_SOURCE_DIR}/kernel)
add_subdirectory(${SimpleKernel_SOURCE_DIR}/drv)
add_subdirectory(${SimpleKernel_SOURCE_DIR}/test)

# Link
add_executable(${KernelName}
        $<TARGET_OBJECTS:libc>
        $<TARGET_OBJECTS:ds_alg>
        $<TARGET_OBJECTS:boot>
        $<TARGET_OBJECTS:gdt>
        $<TARGET_OBJECTS:intr>
        $<TARGET_OBJECTS:debug>
        $<TARGET_OBJECTS:mm>
        $<TARGET_OBJECTS:heap>
        $<TARGET_OBJECTS:sched>
        $<TARGET_OBJECTS:task>
        $<TARGET_OBJECTS:kernel>
        $<TARGET_OBJECTS:mem>
        $<TARGET_OBJECTS:8259A>
        $<TARGET_OBJECTS:clock>
        $<TARGET_OBJECTS:vga>
        $<TARGET_OBJECTS:console>
        $<TARGET_OBJECTS:keyboard>
        $<TARGET_OBJECTS:mouse>
        $<TARGET_OBJECTS:test>)

target_link_options(${KernelName} PRIVATE -T ${SimpleKernel_SOURCE_DIR}/arch/${SimpleKernelArch}/boot/link32.ld)
target_link_options(${KernelName} PRIVATE -Wl,-melf_i386)
