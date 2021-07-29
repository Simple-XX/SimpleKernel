
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch CXX ASM)

find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
set(boot_src ${boot_asm_src})

find_asm_source_files(intr_asm_src ${arch_SOURCE_DIR}/intr)
aux_source_directory(${arch_SOURCE_DIR}/intr intr_cpp_src)
set(intr_src ${intr_asm_src} ${intr_cpp_src})

set(arch_src ${boot_src} ${intr_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})

target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
