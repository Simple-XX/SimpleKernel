
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 设置编译规则

# 根据 SimpleKernelArch 设置编译选项
# 架构相关选项
if (SimpleKernelArch STREQUAL ia32/i386)
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -m32 -Di386")
    set(CMAKE_CXX_FLAGS "-march=corei7 -mtune=corei7 -m32 -Di386")
elseif (SimpleKernelArch STREQUAL ia32/x86_64)
    set(CMAKE_C_FLAGS "-march=corei7 -mtune=corei7 -m64 -mno-red-zone -Dx86_64")
    set(CMAKE_CXX_FLAGS "-march=corei7 -mtune=corei7 -m64 -mno-red-zone -Dx86_64")
elseif (SimpleKernelArch STREQUAL aarch64)
    set(CMAKE_C_FLAGS "-march=armv8-a -mtune=cortex-a72 -D${SimpleKernelArch}")
elseif (SimpleKernelArch STREQUAL riscv64)
    set(CMAKE_C_FLAGS "-march=rv64imafdc -D${SimpleKernelArch}")
    set(CMAKE_CXX_FLAGS "-march=rv64imafdc -D${SimpleKernelArch}")
else ()
    message(FATAL_ERROR "unexpected ARCH: ${SimpleKernelArch}")
endif ()

# 调试相关选项
if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
else ()
    message(WARNING "unexpected  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}, using default value DEBUG")
    set(CMAKE_BUILD_TYPE DEBUG)
endif ()

# 代码优化级别
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# 通用选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -nostdinc -fexceptions -nostartfiles -fPIC  -no-pie -Wall -Wextra -MMD")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -nostdlib -nostdinc -fexceptions -nostartfiles -fPIC  -no-pie -Wall -Wextra -MMD")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# 输出相关信息
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_ASM_FLAGS is ${CMAKE_ASM_FLAGS}")
message(STATUS "TOOLCHAIN_PREFIX is ${TOOLCHAIN_PREFIX}")
message(STATUS "CMAKE_OBJCOPY is ${CMAKE_OBJCOPY}")

# 处理子目录下的 CMakeLists
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/arch)
add_subdirectory(${SimpleKernel_SOURCE_CODE_DIR}/kernel)

# 将各个子对象链接为内核文件
add_executable(${KernelName}
    $<TARGET_OBJECTS:arch>
    $<TARGET_OBJECTS:kernel>)

# 指定链接脚本
target_link_options(${KernelName} PRIVATE -T ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/link.ld)
# 生成 map 文件
target_link_options(${KernelName} PRIVATE -Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KernelName}.map)

# 其它链接选项
if (SimpleKernelArch STREQUAL ia32/i386)
    target_link_options(${KernelName} PRIVATE -Wl,-melf_i386)
elseif (SimpleKernelArch STREQUAL ia32/x86_64)
    target_link_options(${KernelName} PRIVATE -Wl,-melf_x86_64 -Wl,-z,max-page-size=0x1000)
elseif (SimpleKernelArch STREQUAL aarch64)
    target_link_options(${KernelName} PRIVATE -Wl,-maarch64elf)
elseif (SimpleKernelArch STREQUAL riscv64)
    target_link_options(${KernelName} PRIVATE -Wl,-melf64lriscv)
endif ()

# objcopy --only-keep-debug foo foo.dbg 创建一个包含调试信息的文件
add_custom_command(TARGET ${KernelName}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${KernelName} ${KernelName}.debug
        COMMENT "Generating kernel debug...")

# objcopy --strip-debug foo 创建一个去掉调试信息的( strip 的)可执行文件
# objcopy --add-gnu-debuglink=foo.dbg foo 为 strip 的文件添加调试信息链接
add_custom_command(TARGET ${KernelName}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJCOPY} --strip-debug ${KernelName}
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${KernelName}.debug ${KernelName}
        COMMENT "Generating raw kernel...")

# nm
add_custom_command(TARGET ${KernelName}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_NM} -a -A -C -l -n -s --special-syms --synthetic ${KernelName} > ${KernelName}.nm
        COMMENT "nm...")

# readelf -a
add_custom_command(TARGET ${KernelName}
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_READELF} -a ${KernelName} > ${KernelName}.readelf || (exit 0)
    COMMENT "readelf...")

# objdump -D
add_custom_command(TARGET ${KernelName}
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_OBJDUMP} -D ${KernelName} > ${KernelName}.disassembly
    COMMENT "Disassembling...")
