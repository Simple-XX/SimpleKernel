
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        arch
        VERSION 0.0.1
)

enable_language(ASM)
enable_language(CXX)

# 生成对象库
add_library(${PROJECT_NAME} OBJECT
        $<$<STREQUAL:${TARGET_ARCH},riscv64>:
        ${PROJECT_SOURCE_DIR}/${TARGET_ARCH}/boot.S
        >
        ${PROJECT_SOURCE_DIR}/${TARGET_ARCH}/arch_main.cpp
)

# 添加头文件
add_header_arch(${PROJECT_NAME})
add_header_3rd(${PROJECT_NAME})

# 添加编译参数
target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_COMPILE_OPTIONS}
)

# 添加链接参数
target_link_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_LINK_OPTIONS}
)
