
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch CXX ASM)

find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
set(boot_src ${boot_asm_src})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/opensbi openabi_src)

aux_source_directory(${arch_SOURCE_DIR}/pmm pmm_cpp_src)
set(pmm_src ${pmm_cpp_src})

set(arch_src ${boot_src} ${openabi_src} ${pmm_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})

target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
