
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(drv CXX)

if (SimpleKernelArch STREQUAL i386 OR SimpleKernelArch STREQUAL x86_64)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/vga vga_src)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/clock clock_src)
    set(drv_src ${vga_src} ${clock_src})
endif ()

add_library(${PROJECT_NAME} OBJECT ${drv_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
