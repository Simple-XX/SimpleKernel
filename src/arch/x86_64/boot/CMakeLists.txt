
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(boot ASM)

<<<<<<< HEAD
aux_source_directory(${boot_SOURCE_DIR}/. boot_c_src)
find_asm_source_files(ASM_SOURCE_FILES ${boot_SOURCE_DIR})

set(boot_src ${boot_c_src} ${ASM_SOURCE_FILES})
=======
find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
set(boot_src ${boot_asm_src})

set(arch_src ${boot_src})
>>>>>>> intr_timer

add_library(${PROJECT_NAME} OBJECT ${arch_src})

<<<<<<< HEAD
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
=======
>>>>>>> print_only
target_include_kernel_header_files(${PROJECT_NAME})
