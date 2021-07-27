
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch CXX ASM)

if (${SimpleKernelArch} STREQUAL "ia32/i386")
    find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/i386/boot)
    set(boot_src ${boot_asm_src})
elseif (${SimpleKernelArch} STREQUAL "ia32/x86_64")
    find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/x86_64/boot)
    set(boot_src ${boot_asm_src})
endif ()

aux_source_directory(${arch_SOURCE_DIR}/port port_cpp_src)
set(port_src ${port_cpp_src})

set(arch_src ${boot_src} ${port_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})

target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
