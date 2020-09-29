
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

PROJECT(libc C ASM)

aux_source_directory(${libc_SOURCE_DIR}/stdio stdio_src)
aux_source_directory(${libc_SOURCE_DIR}/string string_src)
set(libc_src ${stdio_src} ${string_src})
add_library(${PROJECT_NAME} OBJECT ${libc_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
