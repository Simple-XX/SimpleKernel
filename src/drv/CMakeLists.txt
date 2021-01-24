
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

if (SimpleKernelArch STREQUAL i386 OR SimpleKernelArch STREQUAL x86_64)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vga)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/console)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/clock)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/keyboard)
endif()
