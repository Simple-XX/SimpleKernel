
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        boot
        VERSION 0.0.1
)

enable_language(CXX)

add_executable(${PROJECT_NAME}
        boot.cpp
        load_elf.cpp
        out_stream.cpp
        graphics.cpp
        memory.cpp
)

# 添加头文件
add_header_project(${PROJECT_NAME})
add_header_boot(${PROJECT_NAME})
add_header_3rd(${PROJECT_NAME})

# 添加定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
        ${DEFAULT_BOOT_DEFINITIONS}
)

# 添加编译参数
target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_BOOT_COMPILE_OPTIONS}
)

# 添加链接参数
target_link_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_BOOT_LINK_OPTIONS}
)

# 添加要链接的库
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${DEFAULT_BOOT_LINK_LIB}
)

# 添加依赖
add_dependencies(${PROJECT_NAME}
        ${COMPILE_DEPENDS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${BOOT_ELF_OUTPUT_NAME})

# 获取目标文件信息
objdump_readelf_nm(${PROJECT_NAME})

# 制作 boot.efi
# 将引导调整为 efi
elf2efi(${PROJECT_NAME} ${BOOT_EFI_OUTPUT_NAME})
