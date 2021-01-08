
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(boot ASM)

find_asm_source_files(ASM_SOURCE_FILES ${boot_SOURCE_DIR})

set(boot_src ${ASM_SOURCE_FILES})

add_library(${PROJECT_NAME} OBJECT ${boot_src})

target_include_kernel_header_files(${PROJECT_NAME})