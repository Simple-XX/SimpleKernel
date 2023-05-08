
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 驱动子模块的编译规则

# 设置子模块名与使用的语言
PROJECT(drv CXX)

# 根据 SimpleKernelArch 判断添加哪些文件
# i386 与 x86_64 使用同一套驱动
if (SimpleKernelArch STREQUAL ia32/i386 OR SimpleKernelArch STREQUAL ia32/x86_64)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/tui tui_src)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/multiboot2 multiboot2_src)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/keyboard keyboard_src)
    set(drv_src ${tui_src} ${multiboot2_src} ${keyboard_src})
# arm 驱动
elseif (SimpleKernelArch STREQUAL arm)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/uart uart_src)
    set(drv_src ${uart_src})
# riscv64 驱动
elseif (SimpleKernelArch STREQUAL riscv64)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/opensbi opensbi_src)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/sbi_console sbi_console_src)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/dtb dtb_src)
    set(drv_src ${opensbi_src} ${sbi_console_src} ${dtb_src})
endif ()

# 添加子模块
add_library(${PROJECT_NAME} OBJECT ${drv_src})

# 添加头文件搜索路径
target_include_libc_header_files(${PROJECT_NAME})
target_include_libcxx_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
