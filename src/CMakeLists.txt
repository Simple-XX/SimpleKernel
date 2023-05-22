
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 设置编译规则

project(kernel)
cmake_minimum_required(VERSION 3.13)

# 暂时没有用到的变量，消除警告
set(ignoreMe "${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}${MACHINE}")

# 引入添加头文件函数
include(header_files)
# 引入添加汇编文件函数
include(find_asm_files)

# 输出相关信息
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("CMAKE_C_FLAGS:${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS:${CMAKE_CXX_FLAGS}")
message("CMAKE_ASM_FLAGS:${CMAKE_ASM_FLAGS}")
message("CMAKE_EXE_LINKER_FLAGS:${CMAKE_EXE_LINKER_FLAGS}")
message("ARCH: ${ARCH}")

# 处理子目录下的 CMakeLists
add_subdirectory(${CMAKE_SOURCE_DIR}/arch)
add_subdirectory(${CMAKE_SOURCE_DIR}/kernel)
add_subdirectory(${CMAKE_SOURCE_DIR}/drv)
add_subdirectory(${CMAKE_SOURCE_DIR}/libc)
add_subdirectory(${CMAKE_SOURCE_DIR}/libcxx)

if (ARCH STREQUAL "riscv64")
    # 将各个子对象链接为内核文件
    add_executable(${KernelName}
            $<TARGET_OBJECTS:arch>
            $<TARGET_OBJECTS:kernel>
            $<TARGET_OBJECTS:libc>
            $<TARGET_OBJECTS:libcxx>
            $<TARGET_OBJECTS:drv>
            )
elseif (ARCH STREQUAL "x86_64")
    # 将各个子对象链接为内核文件
    add_executable(${KernelName}
            ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/gnuefi/reloc_${ARCH}.o
            #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/bin/crt0_${ARCH}.o
            $<TARGET_OBJECTS:arch>
            $<TARGET_OBJECTS:kernel>
            $<TARGET_OBJECTS:libc>
            $<TARGET_OBJECTS:libcxx>
            $<TARGET_OBJECTS:drv>
            )
    link_libraries(${KernelName} PRIVATE
            ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/lib/libefi.a
            ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/gnuefi/libgnuefi.a
            #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/bin/libuefi_${ARCH}.a
            )
elseif (ARCH STREQUAL "aarch64")
    # 将各个子对象链接为内核文件
    add_executable(${KernelName}
            ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/gnuefi/reloc_${ARCH}.o
            #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/bin/crt0_${ARCH}.o
            $<TARGET_OBJECTS:arch>
            $<TARGET_OBJECTS:kernel>
            $<TARGET_OBJECTS:libc>
            $<TARGET_OBJECTS:libcxx>
            $<TARGET_OBJECTS:drv>
            )
    link_libraries(${KernelName} PRIVATE
            ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/lib/libefi.a
            ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/gnuefi/libgnuefi.a
            #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/bin/libuefi_${ARCH}.a
            )
endif ()

# 指定链接脚本
target_link_options(${KernelName} PRIVATE -T ${CMAKE_SOURCE_DIR}/arch/${ARCH}/link.ld)

# 其它链接选项
if (ARCH STREQUAL riscv64)
    target_link_options(${KernelName} PRIVATE -Wl,-melf64lriscv)
elseif (ARCH STREQUAL x86_64)
    target_link_options(${KernelName} PRIVATE -Wl,-melf_x86_64)
elseif (ARCH STREQUAL aarch64)
    target_link_options(${KernelName} PRIVATE -Wl,-maarch64elf)
endif ()
