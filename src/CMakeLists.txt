
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# CMakeLists.txt for Simple-XX/SimpleKernel.

# Set C flags for arch
if (SimpleKernelArch STREQUAL "ia32/i386")
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -m32 -D${SimpleKernelArch}")
    set(CMAKE_CXX_FLAGS "-march=corei7 -mtune=corei7 -m32 -D${SimpleKernelArch}")
elseif (SimpleKernelArch STREQUAL "ia32/x86_64")
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -m64 -mno-red-zone -D${SimpleKernelArch}")
    set(CMAKE_CXX_FLAGS "-march=corei7 -mtune=corei7 -m64 -mno-red-zone -D${SimpleKernelArch}")
elseif (SimpleKernelArch STREQUAL arm)
    set(CMAKE_C_FLAGS "-march=armv7-a -mtune=cortex-a7 -marm -D${SimpleKernelArch}")
elseif (SimpleKernelArch STREQUAL riscv64)
    set(CMAKE_C_FLAGS "-D${SimpleKernelArch}")
else ()
    message(FATAL_ERROR "unexpected ARCH: ${SimpleKernelArch}")
endif ()

# Set C flags for debug
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
else ()
    message(WARNING "unexpected  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}, using default value DEBUG")
    set(CMAKE_BUILD_TYPE DEBUG)
endif ()

# Set common flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -nostdinc -fexceptions -nostartfiles -fPIC -O2 -Wall -Wextra -MMD")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -nostdlib -nostdinc -fexceptions -nostartfiles -fPIC -O2 -Wall -Wextra -MMD")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_ASM_FLAGS is ${CMAKE_ASM_FLAGS}")
message(STATUS "TOOLCHAIN_PREFIX is ${TOOLCHAIN_PREFIX}")
message(STATUS "CMAKE_OBJCOPY is ${CMAKE_OBJCOPY}")

# Sub dirs
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/arch)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/kernel)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/drv)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/libc)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/libcxx)

# Link
add_executable(${KernelName}
    $<TARGET_OBJECTS:arch>
    $<TARGET_OBJECTS:libc>
    $<TARGET_OBJECTS:libcxx>
    $<TARGET_OBJECTS:drv>
    $<TARGET_OBJECTS:kernel>)

target_link_options(${KernelName} PRIVATE -T ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/link.ld)

if (SimpleKernelArch STREQUAL "ia32/i386")
    target_link_options(${KernelName} PRIVATE -Wl,-melf_i386)
elseif (SimpleKernelArch STREQUAL "ia32/x86_64")
    target_link_options(${KernelName} PRIVATE -Wl,-melf_x86_64 -Wl,-z,max-page-size=0x1000)
elseif (SimpleKernelArch STREQUAL arm)
    target_link_options(${KernelName} PRIVATE -Wl,-marmelf)
elseif (SimpleKernelArch STREQUAL riscv64)
target_link_options(${KernelName} PRIVATE -Wl,-melf64lriscv)
endif ()
