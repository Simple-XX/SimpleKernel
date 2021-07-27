
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

if (SimpleKernelArch STREQUAL "ia32/i386" OR SimpleKernelArch STREQUAL "ia32/x86_64")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ia32)
else ()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${SimpleKernelArch})
endif ()
