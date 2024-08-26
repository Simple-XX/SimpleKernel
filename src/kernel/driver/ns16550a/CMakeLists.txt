
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

add_library(ns16550a INTERFACE)

target_include_directories(ns16550a INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(ns16550a INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/ns16550a.cpp
)
