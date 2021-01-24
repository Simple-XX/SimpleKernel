
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(arch ASM)

find_asm_source_files(init_asm_src ${arch_SOURCE_DIR}/init)
set(init_src ${init_asm_src})

set(arch_src ${init_src})

add_library(${PROJECT_NAME} OBJECT ${arch_src})

target_include_kernel_header_files(${PROJECT_NAME})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boot)
<<<<<<< HEAD

=======
>>>>>>> print_only
