
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

add_library(libcxx INTERFACE)

target_include_directories(libcxx INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(libcxx INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/sk_libcxx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sk_new.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sk_iostream.cpp
)
