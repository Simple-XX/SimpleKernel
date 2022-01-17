
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 内核子模块的编译规则

# 设置子模块名与使用的语言
PROJECT(kernel CXX ASM)

# 寻找 CXX 文件
aux_source_directory(${kernel_SOURCE_DIR}/. kernel_src)

# 添加子模块
add_library(${PROJECT_NAME} OBJECT ${kernel_src})

# 添加头文件搜索路径
target_include_kernel_header_files(${PROJECT_NAME})
