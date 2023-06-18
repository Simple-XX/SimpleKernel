
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 配置信息

# 跳过编译器检查
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# 设置 cmake 搜索路径
# 仅在 target 搜索 include lib pkg
# 在 host 搜索 program
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置使用的 C/C++ 版本
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# 要运行的平台
set(VALID_MACHINE qemu)
if (NOT DEFINED MACHINE)
    set(MACHINE qemu)
endif ()
message("MACHINE is: ${MACHINE}")

if (NOT MACHINE IN_LIST VALID_MACHINE)
    message(FATAL_ERROR "MACHINE must be one of ${VALID_MACHINE}")
endif ()

# 架构
set(VALID_ARCH riscv64 x86_64 aarch64)
# 默认构建 riscv64
if (NOT DEFINED ARCH)
    set(ARCH riscv64)
endif ()
message("ARCH is: ${ARCH}")

if (NOT ARCH IN_LIST VALID_ARCH)
    message(FATAL_ERROR "ARCH must be one of ${VALID_ARCH}")
endif ()

# 指定要使用的 efi
message("USE_GNU_UEFI is: ${USE_GNU_UEFI}")

# 是否 debug，默认为 Debug
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DEBUG_FLAGS "-Werror")
    #    set(CMAKE_VERBOSE_MAKEFILE OFF)
else ()
    set(CMAKE_BUILD_TYPE "Debug")
    set(DEBUG_FLAGS "-g -ggdb")
    #    set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()
message("CMAKE_BUILD_TYPE is: ${CMAKE_BUILD_TYPE}")

# 代码优化级别
set(OPTIMIZE_FLAGS "-O0")

# 通用编译选项
set(COMMON_FLAGS "-Wall -Wextra \
-no-pie -nostdlib \
-fPIC -ffreestanding -fexceptions -fshort-wchar \
-DUSE_GNU_UEFI=${USE_GNU_UEFI}")

# 架构相关编译选项
# @todo clang 交叉编译参数
if (ARCH STREQUAL "riscv64")
    set(ARCH_FLAGS "-march=rv64imafdc")
elseif (ARCH STREQUAL "x86_64")
    set(ARCH_FLAGS "-march=corei7 -mtune=corei7 -m64 -mno-red-zone \
-z max-page-size=0x1000 \
-Wl,-shared -Wl,-Bsymbolic")
elseif (ARCH STREQUAL "aarch64")
    set(ARCH_FLAGS "-march=armv8-a -mtune=cortex-a72")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTIMIZE_FLAGS} ${COMMON_FLAGS} ${ARCH_FLAGS} ${DEBUG_FLAGS}")
message("CMAKE_C_FLAGS:${CMAKE_C_FLAGS}")

# 将编译选项同步到汇编
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
message("CMAKE_ASM_FLAGS:${CMAKE_ASM_FLAGS}")

# 将编译选项同步到 c++
set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -fpermissive")
message("CMAKE_CXX_FLAGS:${CMAKE_CXX_FLAGS}")

# 设置构建使用的工具，默认为 make
if (GENERATOR STREQUAL "ninja")
    set(GENERATOR_COMMAND ninja)
else ()
    set(GENERATOR_COMMAND make)
endif ()
message("GENERATOR_COMMAND is: ${GENERATOR_COMMAND}")

if (COMPILER STREQUAL "gcc")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_MODULE_PATH}/${ARCH}-${CMAKE_HOST_SYSTEM_PROCESSOR}-gcc.cmake)
elseif (COMPILER STREQUAL "clang")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_MODULE_PATH}/${ARCH}-${CMAKE_HOST_SYSTEM_PROCESSOR}-clang.cmake)
endif ()
message("CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")

# 设置内核名称
set(KernelName kernel.elf)
