
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch ASM)

find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
set(boot_src ${boot_asm_src})

set(arch_src ${boot_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})
