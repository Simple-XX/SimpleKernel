
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch CXX ASM)

if (${SimpleKernelArch} STREQUAL "ia32/i386")
    find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/i386/boot)
    set(boot_src ${boot_asm_src})

    find_asm_source_files(gdt_asm_src ${arch_SOURCE_DIR}/i386/gdt)
    aux_source_directory(${arch_SOURCE_DIR}/i386/gdt gdt_cpp_src)
    set(gdt_src ${gdt_asm_src} ${gdt_cpp_src})

    find_asm_source_files(intr_asm_src ${arch_SOURCE_DIR}/i386/intr)
    aux_source_directory(${arch_SOURCE_DIR}/i386/intr intr_cpp_src)
    set(intr_src ${intr_asm_src}  ${intr_cpp_src})


elseif (${SimpleKernelArch} STREQUAL "ia32/x86_64")
    find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/x86_64/boot)
    set(boot_src ${boot_asm_src})

    find_asm_source_files(gdt_asm_src ${arch_SOURCE_DIR}/x86_64/gdt)
    aux_source_directory(${arch_SOURCE_DIR}/x86_64/gdt gdt_cpp_src)
    set(gdt_src ${gdt_asm_src} ${gdt_cpp_src})

    find_asm_source_files(intr_asm_src ${arch_SOURCE_DIR}/x86_64/intr)
    aux_source_directory(${arch_SOURCE_DIR}/x86_64/intr intr_cpp_src)
    set(intr_src ${intr_asm_src}  ${intr_cpp_src})
endif ()

aux_source_directory(${arch_SOURCE_DIR}/port port_cpp_src)
set(port_src ${port_cpp_src})

aux_source_directory(${arch_SOURCE_DIR}/apic apic_cpp_src)
set(apic_src ${apic_cpp_src})

set(arch_src ${boot_src} ${port_src} ${gdt_src} ${intr_src} ${apic_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})

target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
