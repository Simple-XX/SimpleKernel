
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 设置编译规则

cmake_minimum_required(VERSION 3.13)
project(kernel)

# 暂时没有用到的变量，消除警告
set(ignoreMe "${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}${MACHINE}${USE_GNU_UEFI}")

# 引入添加头文件函数
include(header_files)
# 引入添加汇编文件函数
include(find_asm_files)

message("CMAKE_ASM_FLAGS: ${CMAKE_ASM_FLAGS}")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# 处理子目录下的 CMakeLists
add_subdirectory(${CMAKE_SOURCE_DIR}/arch)
add_subdirectory(${CMAKE_SOURCE_DIR}/kernel)

if (ARCH STREQUAL "riscv64")
    enable_language(ASM)
    # 将各个子对象链接为内核文件
    add_executable(core
            ${CMAKE_SOURCE_DIR}/boot/${ARCH}/boot.S
            $<TARGET_OBJECTS:kernel>
            )
elseif (ARCH STREQUAL "x86_64")
    # 将各个子对象链接为内核文件
    add_executable(core
            $<TARGET_OBJECTS:kernel>
            )

elseif (ARCH STREQUAL "aarch64")
    # 将各个子对象链接为内核文件
    add_executable(core
            $<TARGET_OBJECTS:kernel>
            )
endif ()

# 指定链接脚本
target_link_options(core PRIVATE -T ${CMAKE_SOURCE_DIR}/arch/${ARCH}/link.ld)

# 其它链接选项
if (ARCH STREQUAL "riscv64")
    target_link_options(core PRIVATE -Wl,-melf64lriscv)
elseif (ARCH STREQUAL "x86_64")
    target_link_options(core PRIVATE -Wl,-melf_x86_64)
elseif (ARCH STREQUAL "aarch64")
    target_link_options(core PRIVATE -Wl,-maarch64elf)
endif ()

set_target_properties(core PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
