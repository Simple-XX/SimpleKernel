
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

add_library(libc INTERFACE)

target_include_directories(libc INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(libc INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/sk_libc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sk_string.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sk_stdio.c
)
