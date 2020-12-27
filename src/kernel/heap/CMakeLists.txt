
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(heap C ASM)

aux_source_directory(${heap_SOURCE_DIR}/. heap_src)
add_library(${PROJECT_NAME} OBJECT ${heap_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
