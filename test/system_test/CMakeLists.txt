
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        system-test
        VERSION 0.0.1
)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gnu_efi_test)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/opensbi_test)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    
endif ()
