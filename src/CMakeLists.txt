
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 设置编译规则

cmake_minimum_required(VERSION 3.13)
project(kernel)

# 暂时没有用到的变量，消除警告
set(ignoreMe "${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}${MACHINE}")

# 引入添加头文件函数
include(header_files)
# 引入添加汇编文件函数
include(find_asm_files)

# 处理子目录下的 CMakeLists
add_subdirectory(${CMAKE_SOURCE_DIR}/arch)
add_subdirectory(${CMAKE_SOURCE_DIR}/kernel)

if (ARCH STREQUAL "riscv64")
    # 将各个子对象链接为内核文件
    add_executable(${KernelName}
            $<TARGET_OBJECTS:arch>
            $<TARGET_OBJECTS:kernel>
            )
elseif (ARCH STREQUAL "x86_64")
    # 将各个子对象链接为内核文件
    add_executable(${KernelName}
            $<TARGET_OBJECTS:kernel>
            )

elseif (ARCH STREQUAL "aarch64")
    # 将各个子对象链接为内核文件
    add_executable(${KernelName}
            ${CMAKE_BINARY_DIR}/../3rd/gnu-efi/gnuefi/reloc_${ARCH}.o
            ${CMAKE_BINARY_DIR}/../3rd/gnu-efi/gnuefi/crt0-efi-${ARCH}.o
            #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/crt0.o
            $<TARGET_OBJECTS:arch>
            $<TARGET_OBJECTS:kernel>
            )
    target_link_libraries(${KernelName} PRIVATE
            ${CMAKE_BINARY_DIR}/../3rd/gnu-efi/lib/libefi.a
            ${CMAKE_BINARY_DIR}/../3rd/gnu-efi/gnuefi/libgnuefi.a
            #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/libuefi.a
            )
endif ()

# 指定链接脚本
target_link_options(${KernelName} PRIVATE -T ${CMAKE_SOURCE_DIR}/arch/${ARCH}/link.ld)

# 其它链接选项
if (ARCH STREQUAL "riscv64")
    target_link_options(${KernelName} PRIVATE -Wl,-melf64lriscv)
elseif (ARCH STREQUAL "x86_64")
    target_link_options(${KernelName} PRIVATE -Wl,-melf_x86_64)
elseif (ARCH STREQUAL "aarch64")
    target_link_options(${KernelName} PRIVATE -Wl,-maarch64elf)
endif ()
