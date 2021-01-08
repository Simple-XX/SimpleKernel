
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(port CXX)

aux_source_directory(${port_SOURCE_DIR} port_src)

add_library(${PROJECT_NAME} OBJECT ${port_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})