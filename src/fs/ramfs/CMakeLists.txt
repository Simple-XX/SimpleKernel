
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(ramfs CXX)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} ramfs_src)

add_library(${PROJECT_NAME} OBJECT ${ramfs_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_libcxx_header_files(${PROJECT_NAME})
target_include_fs_header_files(${PROJECT_NAME})
