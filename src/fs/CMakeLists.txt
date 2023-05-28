
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(filesystem CXX)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/fatfs fatfs_src)
set(filesystem_src ${fatfs_src})

add_library(${PROJECT_NAME} OBJECT ${filesystem_src})

target_include_kernel_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_libcxx_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_dev_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_fs_header_files(${PROJECT_NAME})
