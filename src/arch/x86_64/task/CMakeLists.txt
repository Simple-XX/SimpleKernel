
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(task C ASM)

aux_source_directory(${task_SOURCE_DIR}/. task_c_src)
find_asm_source_files(ASM_SOURCE_FILES ${task_SOURCE_DIR})

set(task_src ${task_c_src} ${ASM_SOURCE_FILES})
add_library(${PROJECT_NAME} OBJECT ${task_src})

target_include_ds_alg_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
