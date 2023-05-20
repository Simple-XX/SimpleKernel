
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 设置编译规则

project(kernel)
cmake_minimum_required(VERSION 3.13)

# 引入添加头文件函数
include(header_files)
# 引入添加汇编文件函数
include(find_asm_files)

# 输出相关信息
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message("CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
message("CMAKE_ASM_FLAGS is ${CMAKE_ASM_FLAGS}")
message("TOOLCHAIN_PREFIX is ${TOOLCHAIN_PREFIX}")
message("CMAKE_OBJCOPY is ${CMAKE_OBJCOPY}")

# 处理子目录下的 CMakeLists
add_subdirectory(${CMAKE_SOURCE_DIR}/arch)
add_subdirectory(${CMAKE_SOURCE_DIR}/kernel)

# 将各个子对象链接为内核文件
add_executable(${KernelName}
        ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/gnuefi/reloc_${ARCH}.o
        #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/bin/crt0_${ARCH}.o
        $<TARGET_OBJECTS:arch>
        $<TARGET_OBJECTS:kernel>
        )

link_libraries(${KernelName} PRIVATE
        ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/lib/libefi.a
        ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/bin/${ARCH}/gnuefi/libgnuefi.a
        #        ${CMAKE_SOURCE_DIR}/../3rd/posix-uefi/bin/libuefi_${ARCH}.a
        )

# 指定链接脚本
target_link_options(${KernelName} PRIVATE -T ${CMAKE_SOURCE_DIR}/arch/${ARCH}/link.ld)
# 生成 map 文件
target_link_options(${KernelName} PRIVATE -Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KernelName}.map)

# 其它链接选项
if (ARCH STREQUAL x86_64)
    target_link_options(${KernelName} PRIVATE -Wl,-melf_x86_64 -Wl,-z,max-page-size=0x1000)
elseif (ARCH STREQUAL aarch64)
    target_link_options(${KernelName} PRIVATE -Wl,-maarch64elf)
elseif (ARCH STREQUAL riscv64)
    target_link_options(${KernelName} PRIVATE -Wl,-melf64lriscv)
endif ()

# objcopy --only-keep-debug foo foo.dbg 创建一个包含调试信息的文件
add_custom_target(kernel_debug DEPENDS ${KernelName})
add_custom_command(TARGET kernel_debug
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${KernelName} ${KernelName}.debug
        COMMENT "Generating kernel debug...")

# objcopy --strip-debug foo 创建一个去掉调试信息的( strip 的)可执行文件
# objcopy --add-gnu-debuglink=foo.dbg foo 为 strip 的文件添加调试信息链接
add_custom_target(kernel_raw DEPENDS ${KernelName})
add_custom_command(TARGET kernel_raw
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJCOPY} --strip-debug ${KernelName}
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${KernelName}.debug ${KernelName}
        COMMENT "Generating raw kernel...")

# nm
add_custom_target(kernel_nm DEPENDS ${KernelName})
add_custom_command(TARGET kernel_nm
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_NM} -a -A -C -l -n -s --special-syms --synthetic ${KernelName}
        > ${KernelName}.nm
        COMMENT "nm...")

# readelf -a
add_custom_target(kernel_readelf DEPENDS ${KernelName})
add_custom_command(TARGET kernel_readelf
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_READELF} -a ${KernelName} > ${KernelName}.readelf || (exit 0)
        COMMENT "readelf...")

# objdump -D
add_custom_target(kernel_disassembling DEPENDS ${KernelName})
add_custom_command(TARGET kernel_disassembling
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJDUMP} -D ${KernelName} > ${KernelName}.disassembly
        COMMENT "Disassembling...")
