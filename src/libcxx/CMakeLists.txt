
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(libcxx CXX)

aux_source_directory(${libcxx_SOURCE_DIR}/ libcxx_cxx_src)
find_asm_source_files(ASM_SOURCE_FILES ${libcxx_SOURCE_DIR})

set(libcxx_src ${ASM_SOURCE_FILES} ${libcxx_cxx_src})

add_library(${PROJECT_NAME} OBJECT ${libcxx_src})

target_include_libcxx_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
