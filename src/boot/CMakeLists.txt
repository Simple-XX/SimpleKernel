
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 引导程序

cmake_minimum_required(VERSION 3.13)
project(boot)

# 暂时没有用到的变量，消除警告
set(ignoreMe "${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}${MACHINE}${CMAKE_ASM_FLAGS}${USE_GNU_UEFI}")

enable_language(CXX)

# 引入添加头文件函数
include(header_files)

message("CMAKE_ASM_FLAGS: ${CMAKE_ASM_FLAGS}")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

add_executable(boot
        ${CMAKE_SOURCE_DIR}/${ARCH}/boot.cpp
        )

if (USE_GNU_UEFI STREQUAL "1")
    target_link_libraries(boot PRIVATE
            ${CMAKE_BINARY_DIR}/../../3rd/gnu-efi/gnuefi/crt0-efi-${ARCH}.o
            ${CMAKE_BINARY_DIR}/../../3rd/gnu-efi/gnuefi/reloc_${ARCH}.o
            ${CMAKE_BINARY_DIR}/../../3rd/gnu-efi/gnuefi/libgnuefi.a
            ${CMAKE_BINARY_DIR}/../../3rd/gnu-efi/lib/libefi.a
            )
else ()
    target_link_libraries(boot PRIVATE
            ${CMAKE_BINARY_DIR}/../../3rd/posix-uefi/crt0.o
            ${CMAKE_BINARY_DIR}/../../3rd/posix-uefi/libuefi.a
            )
endif ()

target_link_options(boot PRIVATE -T ${CMAKE_SOURCE_DIR}/${ARCH}/link.ld)

set_target_properties(boot PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

# 添加头文件搜索路径
target_include_gnu_efi_header_files(${PROJECT_NAME})
target_include_posix_uefi_header_files(${PROJECT_NAME})
