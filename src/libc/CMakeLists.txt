
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(libc C CXX)

aux_source_directory(${libc_SOURCE_DIR}/stdio stdio_src)
aux_source_directory(${libc_SOURCE_DIR}/string string_src)
aux_source_directory(${libc_SOURCE_DIR}/math math_src)

set(libc_src ${stdio_src} ${stdio_src} ${string_src} ${math_src})
add_library(${PROJECT_NAME} OBJECT ${libc_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
