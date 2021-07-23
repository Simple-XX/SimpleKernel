
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch CXX ASM)

find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/boot boot_cpp_src)
set(boot_src ${boot_asm_src} ${boot_cpp_src})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/port port_cpp_src)
set(port_src ${port_cpp_src})

find_asm_source_files(gdt_asm_src ${arch_SOURCE_DIR}/gdt)
aux_source_directory(${arch_SOURCE_DIR}/gdt gdt_cpp_src)
set(gdt_src ${gdt_asm_src} ${gdt_cpp_src})

aux_source_directory(${arch_SOURCE_DIR}/pmm pmm_cpp_src)
set(pmm_src ${pmm_cpp_src})

aux_source_directory(${arch_SOURCE_DIR}/vmm vmm_cpp_src)
set(vmm_src ${vmm_cpp_src})

aux_source_directory(${arch_SOURCE_DIR}/apic apic_cpp_src)
set(apic_src ${apic_cpp_src})

find_asm_source_files(intr_asm_src ${arch_SOURCE_DIR}/intr)
aux_source_directory(${arch_SOURCE_DIR}/intr intr_cpp_src)
set(intr_src ${intr_asm_src} ${intr_cpp_src})

set(arch_src ${boot_src} ${port_src} ${gdt_src} ${pmm_src} ${vmm_src} ${apic_src} ${intr_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
