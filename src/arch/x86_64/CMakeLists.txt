
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 架构相关子模块的编译规则

# 设置子模块名
PROJECT(arch)

enable_language(ASM)
enable_language(CXX)

# 寻找汇编文件
find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/boot)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/boot c_cxx_src)

# 添加到 boot_src
set(boot_src ${boot_asm_src} ${c_cxx_src})

# 设置子模块所有的源码
set(arch_src ${boot_src})

# 添加子模块
add_library(${PROJECT_NAME} OBJECT ${arch_src})

# 添加头文件搜索路径
target_include_kernel_header_files(${PROJECT_NAME})
target_include_gnu_efi_header_files(${PROJECT_NAME})
