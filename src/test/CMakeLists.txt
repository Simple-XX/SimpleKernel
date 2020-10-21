
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(test C ASM)

aux_source_directory(${test_SOURCE_DIR}/. test_src)
add_library(${PROJECT_NAME} OBJECT ${test_src})

target_include_ds_alg_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
target_include_tests_header_files(${PROJECT_NAME})