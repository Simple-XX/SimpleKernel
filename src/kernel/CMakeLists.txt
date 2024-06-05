
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

# 添加定义
target_compile_definitions(kernel PRIVATE
        ${DEFAULT_KERNEL_DEFINITIONS}
)

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

# # 获取 crt 文件位置
# execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crti.o OUTPUT_VARIABLE CRTI_O OUTPUT_STRIP_TRAILING_WHITESPACE)
# execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o OUTPUT_VARIABLE CRTBEGIN_O OUTPUT_STRIP_TRAILING_WHITESPACE)
# execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtend.o OUTPUT_VARIABLE CRTEND_O OUTPUT_STRIP_TRAILING_WHITESPACE)
# execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtn.o OUTPUT_VARIABLE CRTN_O OUTPUT_STRIP_TRAILING_WHITESPACE)

# # 设置链接指令
# # set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} <CMAKE_CXX_LINK_FLAGS> <FLAGS> <LINK_FLAGS> ${CRTI_O} ${CRTBEGIN_O} <OBJECTS> <LINK_LIBRARIES> ${CRTEND_O} ${CRTN_O} -o <TARGET>")
# set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} -g -T /home/parallels/github/MRNIU/SimpleKernel_copy/src/kernel/arch/x86_64/link.ld -static -z max-page-size=0x1000 ${CRTI_O} ${CRTBEGIN_O} <OBJECTS> <LINK_LIBRARIES> ${CRTEND_O} ${CRTN_O} -o <TARGET>")

# readelf -a
readelf_a(kernel)

# objdump -D
objdump_D(kernel)
