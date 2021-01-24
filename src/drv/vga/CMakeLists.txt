
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(vga CXX)

aux_source_directory(${vga_SOURCE_DIR}/. vga_src)
add_library(${PROJECT_NAME} OBJECT ${vga_src})

target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
