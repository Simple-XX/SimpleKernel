
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
        memory.cpp
)

# 添加头文件
target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/src
)

# 添加要链接的库
target_link_libraries(${PROJECT_NAME} PRIVATE
        boot_link_libraries
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${BOOT_ELF_OUTPUT_NAME})

# 获取目标文件信息
objdump_readelf_nm(${PROJECT_NAME})

# 制作 boot.efi
# 将引导调整为 efi
elf2efi(${PROJECT_NAME} ${BOOT_EFI_OUTPUT_NAME})
