
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(boot C ASM)

aux_source_directory(${boot_SOURCE_DIR}/. boot_c_src)
find_asm_source_files(ASM_SOURCE_FILES ${boot_SOURCE_DIR})

set(boot_src ${boot_c_src} ${ASM_SOURCE_FILES})

add_library(${PROJECT_NAME} OBJECT ${boot_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
