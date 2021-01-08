
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(intr CXX ASM)

aux_source_directory(${intr_SOURCE_DIR}/. intr_c_src)
find_asm_source_files(ASM_SOURCE_FILES ${intr_SOURCE_DIR})

set(intr_src ${intr_c_src} ${ASM_SOURCE_FILES})
add_library(${PROJECT_NAME} OBJECT ${intr_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
