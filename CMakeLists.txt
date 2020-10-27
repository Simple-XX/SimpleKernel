
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.10)

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set kernel name
set(KernelName kernel.elf)

set(SimpleKernel_SOURCE_CODE_DIR ${SimpleKernel_SOURCE_DIR}/src)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR})
