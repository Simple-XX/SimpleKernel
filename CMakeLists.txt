
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# CMake 入口

# 设置最小 cmake 版本
cmake_minimum_required(VERSION 3.13)

# 跳过编译器检查
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# 设置项目名与使用的语言
project(SimpleKernel LANGUAGES CXX ASM)

# 禁止原地编译
if(${SimpleKernel_SOURCE_DIR} STREQUAL ${SimpleKernel_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# 设置使用的 C/C++ 版本
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 设置辅助 cmake 脚本路径
set(CMAKE_MODULE_PATH "${SimpleKernel_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# 设置 arch
include(${SimpleKernel_SOURCE_DIR}/cmake/arch_detector.cmake)
# 引入添加头文件函数
include(${SimpleKernel_SOURCE_DIR}/cmake/header_files.cmake)
# 引入添加汇编文件函数
include(${SimpleKernel_SOURCE_DIR}/cmake/find_asm_files.cmake)

# 设置输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置内核名称
set(KernelName kernel.elf)

# 设置 src 路径
set(SimpleKernel_SOURCE_CODE_DIR ${SimpleKernel_SOURCE_DIR}/src)
# 跳转到 SimpleKernel_SOURCE_CODE_DIR 下的 CMakeLists
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR})
