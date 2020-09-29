
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(8259A C ASM)

aux_source_directory(${8259A_SOURCE_DIR}/. 8259A_src)
add_library(${PROJECT_NAME} OBJECT ${8259A_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})