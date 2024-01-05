
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
    kernel
    VERSION 0.0.1
)

enable_language(CXX)

add_subdirectory(${PROJECT_SOURCE_DIR}/libc)
add_subdirectory(${PROJECT_SOURCE_DIR}/libcxx)
add_subdirectory(${PROJECT_SOURCE_DIR}/arch)
add_subdirectory(${PROJECT_SOURCE_DIR}/driver)

add_executable(kernel
    main.cpp
)

# 添加头文件
add_header_libc(kernel)
add_header_libcxx(kernel)
add_header_arch(kernel)
add_header_kernel(kernel)
add_header_driver(kernel)
add_header_3rd(kernel)

# 添加编译参数
target_compile_options(kernel PRIVATE
    ${DEFAULT_KERNEL_COMPILE_OPTIONS}
)

# 添加链接参数
target_link_options(kernel PRIVATE
    ${DEFAULT_KERNEL_LINK_OPTIONS}
)

# 添加要链接的库
target_link_libraries(kernel PRIVATE
    ${DEFAULT_KERNEL_LINK_LIB}
    libc
    libcxx
    arch
    driver
)

# 添加依赖
add_dependencies(kernel
    ${COMPILE_DEPENDS}
)

set_target_properties(kernel PROPERTIES PREFIX "")
set_target_properties(kernel PROPERTIES OUTPUT_NAME ${KERNEL_ELF_OUTPUT_NAME})

# readelf -a
readelf_a(kernel)

# objdump -D
objdump_D(kernel)
