
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        kernel
        VERSION 0.0.1
)

enable_language(ASM)
enable_language(C)
enable_language(CXX)

add_subdirectory(${PROJECT_SOURCE_DIR}/libc)
add_subdirectory(${PROJECT_SOURCE_DIR}/libcxx)
add_subdirectory(${PROJECT_SOURCE_DIR}/arch)
add_subdirectory(${PROJECT_SOURCE_DIR}/driver)

add_executable(${PROJECT_NAME}
        main.cpp
)

# 添加头文件
target_include_directories(kernel PRIVATE
        include
)

# 添加要链接的库
target_link_libraries(${PROJECT_NAME} PRIVATE
        kernel_link_libraries
        libc
        libcxx
        arch
        driver
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${KERNEL_ELF_OUTPUT_NAME})

# 获取目标文件信息
objdump_readelf_nm(${PROJECT_NAME})
