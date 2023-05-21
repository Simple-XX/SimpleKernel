
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
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 设置构建使用的工具，默认为 make
if (CMAKE_GENERATOR MATCHES "Ninja")
    set(GENERATOR_COMMAND ninja)
else ()
    set(GENERATOR_COMMAND make)
endif ()

# 要运行的平台
set(VALID_MACHINE qemu)
if (NOT DEFINED MACHINE)
    set(MACHINE qemu)
    message("MACHINE set to: ${MACHINE}")
else ()
    message("MACHINE is: ${MACHINE}")
endif ()

if (NOT MACHINE IN_LIST VALID_MACHINE)
    message(FATAL_ERROR "MACHINE must be one of ${VALID_MACHINE}")
endif ()

# 架构
set(VALID_ARCH riscv64 x86_64 aarch64)
# 默认构建 riscv64
if (NOT DEFINED ARCH)
    set(ARCH riscv64)
    message("ARCH set to: ${ARCH}")
else ()
    message("ARCH is: ${ARCH}")
endif ()

if (NOT ARCH IN_LIST VALID_ARCH)
    message(FATAL_ERROR "ARCH must be one of ${VALID_ARCH}")
endif ()

# 是否 debug，默认为 Debug
if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_VERBOSE_MAKEFILE OFF)
else ()
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb")
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

# 代码优化级别
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

# 通用编译选项
set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} \
-Wall -Wextra \
-fPIC -ffreestanding -fexceptions -fshort-wchar \
-DGNU_EFI_USE_MS_ABI")

# 架构相关编译选项
if (ARCH STREQUAL "riscv64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv64imafdc")
elseif (ARCH STREQUAL "x86_64")
    set(CMAKE_C_FLAGS
            "${CMAKE_C_FLAGS} -march=corei7 -mtune=corei7 -m64 -mno-red-zone")
elseif (ARCH STREQUAL "aarch64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mtune=cortex-a72")
endif ()

# 将编译选项同步到汇编
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# 将编译选项同步到 c++
set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -fpermissive")

# 链接选项
# 生成 map 文件
# 指定链接脚本
set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} \
-z max-page-size=0x1000 -nostdlib -shared -Wl,-Bsymbolic")

# 设置内核名称
set(KernelName kernel.elf)
