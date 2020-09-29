
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(sched C ASM)

aux_source_directory(${sched_SOURCE_DIR}/. sched_c_src)
find_asm_source_files(ASM_SOURCE_FILES ${sched_SOURCE_DIR})

set(sched_src ${sched_c_src} ${ASM_SOURCE_FILES})

add_library(${PROJECT_NAME} OBJECT ${sched_src})

target_include_ds_alg_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
