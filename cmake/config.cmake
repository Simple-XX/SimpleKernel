
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 配置信息

# 跳过编译器检查
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# 设置使用的 C/C++ 版本
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

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

# 是否 debug，默认为发布版
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
else ()
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif ()
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

# 设置构建使用的工具，默认为 make
if (CMAKE_GENERATOR MATCHES "Ninja")
    set(GENERATOR_COMMAND ninja)
else ()
    set(GENERATOR_COMMAND make)
endif ()

# 代码优化级别
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# 通用编译选项
set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fexceptions \
-fPIC -DGNU_EFI_USE_MS_ABI -fshort-wchar -Wl,-Bsymbolic \
-Wl,-shared -Wall -Wextra -MMD")
set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fpermissive -ffreestanding -nostdlib \
-fexceptions -fPIC -DGNU_EFI_USE_MS_ABI -fshort-wchar -Wl,-Bsymbolic \
-Wl,-shared -Wall -Wextra -MMD")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置内核名称
set(KernelName kernel.elf)
