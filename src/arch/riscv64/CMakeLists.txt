
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

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/opensbi openabi_src)
# 设置子模块所有的源码
set(arch_src ${boot_src} ${openabi_src})

# 添加子模块
add_library(${PROJECT_NAME} OBJECT ${arch_src})

# 添加头文件搜索路径
target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})

