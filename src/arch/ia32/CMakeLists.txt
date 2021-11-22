
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 架构相关子模块的编译规则

# 设置子模块名与使用的语言
PROJECT(arch CXX ASM)

# 根据 SimpleKernelArch 判断添加哪些文件
# 32 位
if (${SimpleKernelArch} STREQUAL "ia32/i386")
    # 寻找汇编文件
    find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/i386/boot)
    # 添加到 boot_src
    set(boot_src ${boot_asm_src})

    find_asm_source_files(gdt_asm_src ${arch_SOURCE_DIR}/i386/gdt)
    aux_source_directory(${arch_SOURCE_DIR}/i386/gdt gdt_cpp_src)
    set(gdt_src ${gdt_asm_src} ${gdt_cpp_src})

# 64 位
elseif (${SimpleKernelArch} STREQUAL "ia32/x86_64")
    # 寻找汇编文件
    find_asm_source_files(boot_asm_src ${arch_SOURCE_DIR}/x86_64/boot)
    # 添加到 boot_src
    set(boot_src ${boot_asm_src})

    find_asm_source_files(gdt_asm_src ${arch_SOURCE_DIR}/x86_64/gdt)
    aux_source_directory(${arch_SOURCE_DIR}/x86_64/gdt gdt_cpp_src)
    set(gdt_src ${gdt_asm_src} ${gdt_cpp_src})
endif ()

aux_source_directory(${arch_SOURCE_DIR}/port port_cpp_src)
# 添加到 port_src
set(port_src ${port_cpp_src})
# 设置子模块所有的源码
set(arch_src ${boot_src} ${port_src} ${gdt_src})

# 添加子模块
add_library(${PROJECT_NAME} OBJECT ${arch_src})

# 添加头文件搜索路径
target_include_arch_header_files(${PROJECT_NAME})
target_include_libc_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
