
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 架构相关子模块的编译规则

# 设置子模块名与使用的语言
PROJECT(arch CXX ASM)

# 寻找汇编文件
find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
# 添加到 boot_src
set(boot_src ${boot_asm_src})

find_asm_source_files(intr_asm_src ${arch_SOURCE_DIR}/intr)
aux_source_directory(${arch_SOURCE_DIR}/intr intr_cpp_src)
set(intr_src ${intr_asm_src} ${intr_cpp_src})

find_asm_source_files(task_asm_src ${arch_SOURCE_DIR}/task)
aux_source_directory(${arch_SOURCE_DIR}/task task_cpp_src)
set(task_src ${task_asm_src} ${task_cpp_src})

set(arch_src ${boot_src} ${intr_src} ${task_src})

# 添加子模块
add_library(${PROJECT_NAME} OBJECT ${arch_src})

# 添加头文件搜索路径
target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_libcxx_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
