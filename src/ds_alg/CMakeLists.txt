
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(ds_alg C ASM)

aux_source_directory(${ds_alg_SOURCE_DIR}/. ds_alg_src)
add_library(${PROJECT_NAME} OBJECT ${ds_alg_src})

target_include_ds_alg_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
