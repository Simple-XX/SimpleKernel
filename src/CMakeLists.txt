
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# riscv64 不支持 uefi
if (${TARGET_ARCH} STREQUAL "x86_64" OR ${TARGET_ARCH} STREQUAL "aarch64")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boot)
endif ()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/kernel)
