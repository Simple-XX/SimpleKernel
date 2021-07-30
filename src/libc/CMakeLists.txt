
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(libc C)

aux_source_directory(${libc_SOURCE_DIR}/src/stdio stdio_src)
aux_source_directory(${libc_SOURCE_DIR}/src/stdlib stdlib_src)
aux_source_directory(${libc_SOURCE_DIR}/src/string string_src)
aux_source_directory(${libc_SOURCE_DIR}/src/math math_src)

set(libc_src ${stdio_src} ${stdio_src} ${stdlib_src} ${string_src} ${math_src})
add_library(${PROJECT_NAME} OBJECT ${libc_src})

target_include_libc_header_files(${PROJECT_NAME})
