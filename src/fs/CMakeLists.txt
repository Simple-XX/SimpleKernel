
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(filesystem CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ramfs)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fat32)
