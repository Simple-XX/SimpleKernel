
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(io CXX)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/vga vga_src)

set(io_src ${vga_src})

add_library(${PROJECT_NAME} OBJECT ${io_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
