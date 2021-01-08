
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(libc++ C CXX ASM)

aux_source_directory(${libc++_SOURCE_DIR}/ libc++_cxx_src)
find_asm_source_files(ASM_SOURCE_FILES ${libc++_SOURCE_DIR})

set(libc++_src ${ASM_SOURCE_FILES} ${libc++_cxx_src})

add_library(${PROJECT_NAME} OBJECT ${libc++_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
